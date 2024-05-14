#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#define CURL_STATICLIB

#include <iostream>
#include <asio.hpp>
#include <chrono>
#include <queue>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <random>
#include <fstream>

#include "node.h"
#include "connection.h"

using namespace std;

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
};

static size_t writeData(void* buffer, size_t size, size_t nmemb, void* userp) {return size * nmemb;}

static int generateRandom(int low, int high) {
	static random_device rd;
	static mt19937 gen(rd());
	uniform_int_distribution<int> distribution(low, high);
	return distribution(gen);
}

Node::Node() {
	dht = DHT();

	//ip = getIP();
	//string root = getDDNS();
	
	ip = "127.0.0.1"; // lan
	string root = "127.0.0.1";

	cout << "IP: " << ip << endl;
	cout << "Root: " << root << endl;

	if (ip == root && false) { // lan
		becomeRoot();
	}
	else if (!connectToRoot()) {
		setDDNS(ip);
		becomeRoot();
	}

	if (!is_root) {
		connect();
	}
	
	handle_thread = thread(&Node::handleThread, this);
	keepalive_thread = thread(&Node::keepalive, this);
	lookout_thread = thread(&Node::lookout, this);
}

string Node::getIP() {
	return "127.0.0.1"; // lan

	CURL* curl;
	CURLcode res;
	string response;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org?format=json");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl; // debug

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return nlohmann::json::parse(response)["ip"];
}

string Node::getDDNS() {
	return "127.0.0.1"; // lan

	CURL* curl;
	CURLcode res;
	string response;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.dynu.com/v2/dns");

		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Host: api.dynu.com");
		headers = curl_slist_append(headers, "accept: application/json");
		headers = curl_slist_append(headers, ("API-Key: " + DDNS_API_KEY).c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl; // debug

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return nlohmann::json::parse(response)["domains"][0]["ipv4Address"];
}

void Node::setDDNS(string ip) {
	return; // lan
	
	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, ("https://api.dynu.com/v2/dns/" + DDNS_ID).c_str());

		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Host: api.dynu.com");
		headers = curl_slist_append(headers, "accept: application/json");
		headers = curl_slist_append(headers, ("API-Key: " + DDNS_API_KEY).c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		string json_data = "{\"name\":\"" + DDNS_URL + "\",\"ipv4Address\":\"" + ip + "\"}";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl; // debug

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
}

void Node::becomeRoot() {
	is_root = true;
	root_node = make_shared<RootNode>(shared_ptr<Node>(this));
}

void Node::connect() {
	string message = "rpnp\ndht join\ndht request";
	rootConnection->writeData(message);
}

void Node::handleThread() {
	while (true) {
		if (rootConnection) {
			if (rootConnection->incoming_messages.size() > 0) {
				string message = rootConnection->incoming_messages.front();
				rootConnection->incoming_messages.pop();

				handleMessage(rootConnection, message);
			}
		}

		unique_lock<mutex> punchholeRC_lock(punchholeRC_mutex);
		if (punchholeRC) {
			if (punchholeRC->incoming_messages.size() > 0) {
				string message = punchholeRC->incoming_messages.front();
				punchholeRC->incoming_messages.pop();
				punchholeRC_lock.unlock();

				handleMessage(punchholeRC, message);
			}
		}
		if (punchholeRC_lock.owns_lock()) punchholeRC_lock.unlock();
		
		vector<shared_ptr<Connection>> copied_connections;
		copyConnections(copied_connections);
		for (shared_ptr<Connection>& connection : copied_connections) {
			if (connection) {
				if (connection->incoming_messages.size() > 0) {
					string message = connection->incoming_messages.front();
					connection->incoming_messages.pop();

					handleMessage(connection, message);
				}
			}
		}

		this_thread::sleep_for(chrono::milliseconds(HANDLE_FREQUENCY));
	}
}

int Node::pickNodeToConnect() {
	// random, this defines the structure of the network
	// keep some randomness to not get stuck in a loop if someones not working
	vector<shared_ptr<DHTNode>> nodes;
	vector<int> bad = {id};

	unique_lock<mutex> connections_lock(connections_mutex);
	for (shared_ptr<Connection>& connection : connections) {
		bad.push_back(connection->id);
	}
	connections_lock.unlock();

	lock_guard<mutex> lock(dht.nodes_mutex);
	for (shared_ptr<DHTNode>& node : dht.nodes) {
		if (node->level == -1) bad.push_back(node->id);
	}

	for (shared_ptr<DHTNode>& node : dht.nodes) {
		if (find(bad.begin(), bad.end(), node->id) != bad.end()) continue;
		if (node->connections.size() >= 3) continue;

		nodes.push_back(node);
	}

	if (connections.size() < 2) {
		if (nodes.size() == 0) {
			for (shared_ptr<DHTNode>& node : dht.nodes) {
				if (find(bad.begin(), bad.end(), node->id) != bad.end()) continue;
				if (node->id == dht.nodes[0]->id) continue;
				if (node->connections.size() > 3) continue;

				nodes.push_back(node);
			}
		}

		if (nodes.size() == 0) {
			for (shared_ptr<DHTNode>& node : dht.nodes) {
				if (find(bad.begin(), bad.end(), node->id) != bad.end()) continue;
				if (node->id == dht.nodes[0]->id) continue;

				nodes.push_back(node);
			}
		}
	}

	if (nodes.size() == 0) return -1;

	int index = generateRandom(0, nodes.size() - 1);

	return nodes[index]->id;
}

void Node::manageConnections() { // make this on thread
	if (is_root) return;
	if (connections.size() == 3) return;

	while (connections.size() > 3) {
		int before = connections.size();

		int my_level = dht.getNodeFromId(id)->level;
		bool parent_found = false;

		unique_lock<mutex> lock(connections_mutex);
		for (int i = 0; i < connections.size() - 1; i++) {
			if (connections[i]->id == dht.nodes[0]->id) continue;
			if (dht.getNodeFromId(connections[i]->id)->connections.size() == 1) continue;
			if (connections[i]->chess_connection) continue;

			if (parent_found || dht.getNodeFromId(connections[i]->id)->level >= my_level) {
				lock.unlock();
				disconnect(connections[i]->id);
				break;
			}
			else parent_found = true;
		}

		if (before == connections.size()) break;
	}

	if (connections.size() < 3 && !connecting) {
		int pick = pickNodeToConnect();
		if (pick == -1) return;

		cout << to_string(id) << " -> " << to_string(pick) << endl;

		lock_guard<mutex> lock(punchholeRC_mutex);
		connecting = true;
		if (connectToPunchholeRoot()) {
			connecting_id = pick;

			string message = "rpnp\npunchhole request " + to_string(id) + " " + to_string(pick);
			punchholeRC->writeData(message);
		}
		else if (!punchholeRC) {
			connecting = false;
		}
	}
}

void Node::rerootCheck() {
	is_reroot = true;

	while (dht.nodes.size() >= 2 && dht.nodes[1]->id == id) {
		Connection connection(dht.nodes[0]->ip, ROOT_PORT, dht.nodes[0]->id);

		if (!connection.connected && !is_root) {
			dht.deleteNode(dht.nodes[0]->id);
			setDDNS(dht.nodes[0]->ip);
			becomeRoot();

			is_reroot = false;
			return;
		}

		this_thread::sleep_for(chrono::seconds(REROOT_CHECK_FREQUENCY));
	}

	is_reroot = false;
}

void Node::lookout() {
	while (true) {
		time_point now = chrono::high_resolution_clock::now();
		vector<int> bad_ids;

		vector<shared_ptr<Connection>> copied_connections;
		copyConnections(copied_connections);
		for (shared_ptr<Connection>& connection : copied_connections) {
			int time_passed = chrono::duration_cast<chrono::seconds>(now - connection->keepalive).count();
			if (time_passed > KEEPALIVE_DETECTION) {
				bad_ids.push_back(connection->id);
			}
		}

		for (int& bad_id : bad_ids) {
			disconnect(bad_id);
		}

		for (int i = 0; i < relay_sessions.size(); i++) {
			int time_passed = chrono::duration_cast<chrono::seconds>(now - relay_sessions[i].creation).count();
			if (time_passed > SESSION_TTL) {
				relay_sessions.erase(relay_sessions.begin() + i);
				i--;
			}
		}

		if (punchholeRC && connecting) {
			int time_passed = chrono::duration_cast<chrono::seconds>(now - punchholeRC_creation).count();
			if (time_passed > PUNCHHOLERC_TTL + generateRandom(0, PUNCHHOLERC_TTL_JITTER)) {
				unique_lock<mutex> punchholeRC_lock(punchholeRC_mutex);
				punchholeRC.reset();
				punchholeRC_lock.unlock();
				connecting = false;

				manageConnections();
			}
		}

		if (connections.size() == 0 && !is_root) {
			if (in_network && dht.connections.size() > 0) {
				in_network = false;
				rootConnection.reset();
				unique_lock<mutex> lock(punchholeRC_mutex);
				punchholeRC.reset();
				connecting = false;

				if (connectToRoot()) {
					if (id != -1) {
						string message = "rpnp\ndht leave " + to_string(id);
						rootConnection->writeData(message);
					}

					connect();
				}
				lock.unlock();
			}
		}

		if (joined_time) {
			int time_passed = chrono::duration_cast<chrono::seconds>(now - *joined_time).count();
			if (time_passed > DETACHED_DETECTION) {
				rootConnection.reset();
				unique_lock<mutex> lock(punchholeRC_mutex);
				punchholeRC.reset();
				lock.unlock();
				connecting = false;

				if (connectToRoot()) {
					connect();
				}
			}
		}

		if (dht.nodes.size() > 0 && dht.nodes[0]->id == id && !is_root) {
			setDDNS(dht.nodes[0]->ip);
			becomeRoot();
		}

		if (chess_connection && !chess_connection->chess_connection) chess_connection.reset();

		this_thread::sleep_for(chrono::milliseconds(LOOKOUT_CHECK_FREQUENCY));
	}
}

void Node::keepalive() {
	while (true) {
		string message = "keepalive";

		vector<shared_ptr<Connection>> copied_connections;
		copyConnections(copied_connections);
		for (shared_ptr<Connection>& connection : copied_connections) {
			if (connection->connected) connection->writeProtocolless(message);
		}

		if (rootConnection) {
			rootConnection->writeProtocolless(message);
		}

		unique_lock<mutex> lock(punchholeRC_mutex);
		if (punchholeRC) {
			punchholeRC->writeProtocolless(message);
		}
		lock.unlock();

		this_thread::sleep_for(chrono::seconds(KEEPALIVE_FREQUENCY));
	}
}

bool Node::connectToRoot() {
	if (rootConnection) return true;

	string root;
	if (dht.nodes.size() == 0) root = getDDNS();
	else root = dht.nodes[0]->ip;

	rootConnection = make_shared<Connection>(root, ROOT_PORT, 0);

	if (rootConnection->connected) return true;
	else {
		rootConnection.reset();

		return false;
	}
}

bool Node::connectToPunchholeRoot() {
	if (punchholeRC) return false;

	string root;
	if (dht.nodes.size() == 0) root = getDDNS();
	else root = dht.nodes[0]->ip;

	punchholeRC = make_shared<Connection>(root, ROOT_PORT, 0);

	if (punchholeRC->connected) {
		punchholeRC_creation = chrono::high_resolution_clock::now();

		return true;
	}
	else {
		punchholeRC.reset();

		return false;
	}
}

void Node::punchholeConnect(string target_ip, int target_port, int target_id) {
	unique_lock<mutex> punchholeRC_lock(punchholeRC_mutex);
	shared_ptr<Connection> connection = punchholeRC;
	punchholeRC.reset();
	punchholeRC_lock.unlock();

	connection->change(target_ip, target_port, target_id, private_key);
	
	if (connection->connected) {
		unique_lock<mutex> connections_lock(connections_mutex);
		connections.push_back(connection);
		connections_lock.unlock();

		shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(id), dht.getNodeFromId(target_id));
		dht.addConnection(dht_connection);

		connection->encryption_key = retrieveKey(target_id);
		connection->decryption_key = private_key;

		string message = "rpnp\ndht connect " + to_string(id) + " " + to_string(target_id);
		relay(dht.nodes[0]->id, message);
		
		cout << to_string(id) << " == " << to_string(target_id) << endl;

		in_network = true;
		joined_time.reset();
	}
	else {
		connection.reset();
	}

	connecting = false;

	manageConnections();
}

void Node::simulatedPunchholeConnect(string target_ip, int target_port, int target_id) {
	unique_lock<mutex> punchholeRC_lock(punchholeRC_mutex);
	punchholeRC.reset();
	punchholeRC_lock.unlock();

	shared_ptr<Connection> connection = make_shared<Connection>(target_ip, target_port, target_id, private_key);

	if (connection->connected) {
		unique_lock<mutex> connections_lock(connections_mutex);
		connections.push_back(connection);
		connections_lock.unlock();

		shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(id), dht.getNodeFromId(target_id));
		dht.addConnection(dht_connection);

		connection->encryption_key = retrieveKey(target_id);
		connection->decryption_key = private_key;

		string message = "rpnp\ndht connect " + to_string(id) + " " + to_string(target_id);
		relay(dht.nodes[0]->id, message);

		cout << to_string(id) << " == " << to_string(target_id) << endl;

		in_network = true;
		joined_time.reset();
	}
	else {
		connection.reset();
	}

	connecting = false;

	manageConnections();
}

vector<int> Node::findPathToRoot() {
	vector<int> path {id};
	int level = dht.getNodeFromId(id)->level;
	shared_ptr<DHTNode> current_node = dht.getNodeFromId(id);

	while (level != 0) {
		for (shared_ptr<DHTConnection>& connection : current_node->connections) {
			if (*connection->a == *current_node && connection->b->level < level) {
				current_node = connection->b;
				path.push_back(current_node->id);
				level = current_node->level;

				break;
			}
			
			if (*connection->b == *current_node && connection->a->level < level) {
				current_node = connection->a;
				path.push_back(current_node->id);
				level = current_node->level;

				break;
			}
		}
	}

	return path;
}

vector<int> Node::findPath(int target_id) {
	vector<int> been = {id};
	queue<vector<int>> queue;
	queue.push(been);

	while (!queue.empty()) {
		vector<int> current_vector = queue.front();
		queue.pop();
		shared_ptr<DHTNode> current_node = dht.getNodeFromId(current_vector.back());

		for (shared_ptr<DHTConnection>& connection : current_node->connections) {
			shared_ptr<DHTNode> next_node;
			if (*connection->a == *current_node) next_node = connection->b;
			else next_node = connection->a;

			bool been_check = false;
			for (int i = 0; i < been.size(); i++) {
				if (been[i] == next_node->id) {
					been_check = true;
					break;
				}
			}

			if (!been_check) {
				vector<int> new_vector = current_vector;
				new_vector.push_back(next_node->id);
				
				if (next_node->id == target_id) return new_vector;

				queue.push(new_vector);
				been.push_back(next_node->id);
			}
		}
	}

	return vector<int>();
}

void Node::relay(int target_id, string payload) {
	if (is_root && target_id == id) {
		root_node->handleMessage(payload);
		return;
	}

	vector<int> path;
	if (target_id == dht.nodes[0]->id) path = findPathToRoot();
	else path = findPath(target_id);
	
	if (path.size() <= 1) return;

	string message = payload;
	int session = generateRandom(0, 999999);
	string key;

	if (path.size() > 2) {
		pair<string, string> message_key_pair = Encryption::encryptVerbose(message, retrieveKey(path[path.size() - 1]));
		message = message_key_pair.first;
		key = message_key_pair.second;
		
		message = "pnp\nrelay request " + to_string(path[path.size() - 1]) + " " + to_string(path[path.size() - 2]) + " " + to_string(session) + "\n" + message;

		for (int i = path.size() - 1; i > 1; i--) {
			message = Encryption::encrypt(message, retrieveKey(path[i]));
			message = "epnpr\n" + message + "\nepnpr";

			message = "pnp\nrelay request " + to_string(path[i]) + " " + to_string(path[i - 2]) + " " + to_string(session) + "\n" + message;
		}
	}

	unique_lock<mutex> lock(connections_mutex);
	for (shared_ptr<Connection>& connection : connections) {
		if (connection->id == path[1]) {
			connection->writeData(message);
			
			break;
		}
	}
	lock.unlock();

	if (path.size() > 2) {
		RelaySession relay_session = RelaySession(target_id, id, session, key);
		relay_sessions.push_back(relay_session);
	}
}

void Node::disconnect(int target_id) {
	string message = "pnp\ndisconnect " + to_string(id);
	shared_ptr<Connection> connection;

	unique_lock<mutex> lock(connections_mutex);
	for (int i = 0; i < connections.size(); i++) {
		if (connections[i]->id == target_id) {
			connection = connections[i];
			connections[i]->writeData(message);
			connections.erase(connections.begin() + i);

			break;
		}
	}
	lock.unlock();

	shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(id), dht.getNodeFromId(target_id));
	dht.deleteConnection(dht_connection);
	
	if (is_root || dht.getNodeFromId(id)->connections.size() > 0) {
		message = "rpnp\ndht disconnect " + to_string(id) + " " + to_string(target_id);
		relay(dht.nodes[0]->id, message);
	}

	if (is_root && connection) {
		int port = connection->socket->local_endpoint().port();
		
		connection->socket->close();
		connection.reset();

		if (port - ROOT_PORT - 1 >= 0 && port - ROOT_PORT - 1 < root_node->port_use.size()) {
			root_node->port_use[port - ROOT_PORT - 1] = false;
		}
	}
	else connection.reset();
}

void Node::copyConnections(vector<shared_ptr<Connection>>& copy) {
	lock_guard<mutex> lock(connections_mutex);

	copy.reserve(connections.size());

	for (shared_ptr<Connection> connection : connections) {
		copy.push_back(shared_ptr<Connection>(connection));
	}
}

void Node::changeName(string new_name) {
	string message = "rpnp\ndht rename " + to_string(id) + " " + new_name;
	relay(dht.nodes[0]->id, message);

	name = new_name;

	Storage::setName(new_name);
}

void Node::sendInvite(int target_id) {
	outgoing_invite = make_shared<ChessInvite>(target_id, id, generateRandom(0, 999999));

	string message = "pnp\nchess invite " + to_string(id) + " " + to_string(outgoing_invite->game);
	relay(target_id, message);
}

void Node::cancelInvite() {
	if (!outgoing_invite) return;

	string message = "pnp\nchess cancel " + to_string(outgoing_invite->game);
	relay(outgoing_invite->to, message);

	outgoing_invites_history.push_back(outgoing_invite);
	outgoing_invite.reset();
}

void Node::acceptInvite() {
	if (incoming_invites.size() == 0) return;

	string message = "pnp\nchess accept " + to_string(incoming_invites[0]->game);
	relay(incoming_invites[0]->from, message);

	incoming_invites.erase(incoming_invites.begin());
}

void Node::rejectInvite() {
	if (incoming_invites.size() == 0) return;

	string message = "pnp\nchess reject " + to_string(incoming_invites[0]->game);
	relay(incoming_invites[0]->from, message);

	incoming_invites.erase(incoming_invites.begin());
}

bool Node::checkConnectedToNode(int target_id) {
	vector<shared_ptr<Connection>> copied_connections;
	copyConnections(copied_connections);

	for (int i = 0; i < copied_connections.size(); i++) {
		if (copied_connections[i]->id == target_id) return true;
	}

	return false;
}

shared_ptr<Connection> Node::getConnectionToNode(int target_id) {
	vector<shared_ptr<Connection>> copied_connections;
	copyConnections(copied_connections);
	
	for (int i = 0; i < copied_connections.size(); i++) {
		if (copied_connections[i]->id == target_id) return copied_connections[i];
	}

	return nullptr;
}

void Node::createGame(int game) {
	shared_ptr<ChessInvite> game_invite;
	if (outgoing_invite && outgoing_invite->game == game) game_invite = outgoing_invite;
	else {
		for (int i = 0; i < outgoing_invites_history.size(); i++) {
			if (outgoing_invites_history[i]->game == game) {
				game_invite = outgoing_invites_history[i];
				break;
			}
		}
	}

	if (!game_invite) return;
	
	outgoing_invite.reset();
	outgoing_invites_history.clear();
	
	while (!checkConnectedToNode(game_invite->to)) {
		if (!connecting) {
			lock_guard<mutex> lock(punchholeRC_mutex);
			connecting = true;
			cout << to_string(id) << " -> " << to_string(game_invite->to) << endl;
			if (connectToPunchholeRoot()) {
				connecting_id = game_invite->to;

				string message = "rpnp\npunchhole request " + to_string(id) + " " + to_string(game_invite->to);
				punchholeRC->writeData(message);
			}
			else if (!punchholeRC) {
				connecting = false;
			}
		}

		this_thread::sleep_for(chrono::milliseconds(HANDLE_FREQUENCY));
	}
	
	shared_ptr<Connection> connection = getConnectionToNode(game_invite->to);
	connection->chess_connection = true;

	int me = game_invite->game % 2 ^ (game_invite->from > game_invite->to);
	string white_player = me ? dht.getNodeFromId(game_invite->from)->name : dht.getNodeFromId(game_invite->to)->name;
	string black_player = !me ? dht.getNodeFromId(game_invite->from)->name : dht.getNodeFromId(game_invite->to)->name;

	string message = "pnp\nchess start " + to_string(game_invite->game);
	connection->writeData(message);

	lock_guard<mutex> lock(board_mutex);
	connection->board = make_shared<Board>(connection, me, white_player, black_player);

	chess_connection = connection;
}

void Node::createKeys() {
	pair<shared_ptr<RSA>, shared_ptr<RSA>> keys = Encryption::generateRSAKeyPair();

	public_key = keys.first;
	private_key = keys.second;

	string key_pem = Encryption::exportRSAPublicKey(public_key);

	if (!is_root) {
		string message = "rpnp\nbroadcast\nkey share " + to_string(id) + "\n" + key_pem;
		rootConnection->writeData(message);
	}
}

shared_ptr<RSA> Node::retrieveKey(int key_id) {
	if (key_id == id) return public_key;

	string key_pem = Storage::getKey(key_id);
	if (!key_pem.empty()) return Encryption::importRSAPublicKey(key_pem);

	shared_ptr<Connection> lowest_level = connections[0];
	for (shared_ptr<Connection>& connection : connections) {
		if (dht.getNodeFromId(connection->id)->level < dht.getNodeFromId(lowest_level->id)->level) lowest_level = connection;
	}

	string message = "pnp\nkey query " + to_string(key_id);
	lowest_level->writeData(message);

	int i = 0;
	while (i < 100) {
		this_thread::sleep_for(chrono::seconds(KEY_RETRIEVE_CHECK_FREQUENCY));

		key_pem = Storage::getKey(key_id);
		if (!key_pem.empty()) return Encryption::importRSAPublicKey(key_pem);

		i++;
	}

	return nullptr;
}

RelaySession::RelaySession(int to, int from, int session) : to(to), from(from), session(session) {
	creation = chrono::high_resolution_clock::now();
}

RelaySession::RelaySession(int to, int from, int session, string k) : RelaySession(to, from, session) {
	creation = chrono::high_resolution_clock::now();
	key = k;
}

ChessInvite::ChessInvite(int to, int from, int game) : to(to), from(from), game(game) {}