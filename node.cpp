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

	string ip = getIP();
	string root = getDDNS();
	
	// try to connect, if cant, become root
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
	// lan
	return "127.0.0.1";

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
	// lan
	return "127.0.0.1";

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
	// lan
	return;

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
	root_node =  make_shared<RootNode>(shared_ptr<Node>(this));
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

		if (punchholeRC) {
			if (punchholeRC->incoming_messages.size() > 0) {
				string message = punchholeRC->incoming_messages.front();
				punchholeRC->incoming_messages.pop();

				handleMessage(punchholeRC, message);
			}
		}
		
		//for (shared_ptr<Connection>& connection : connections) {
		for (int i = 0; i < connections.size(); i++) {
			shared_ptr<Connection>& connection = connections[i];
			if (!connection) continue; // stupid
			if (connection->incoming_messages.size() > 0) {
				string message = connection->incoming_messages.front();
				connection->incoming_messages.pop();

				handleMessage(connection, message);
			}
		}

		this_thread::sleep_for(chrono::milliseconds(HANDLE_FREQUENCY));
	}
}

int Node::pickNodeToConnect() {
	// random implementation, this defines the structure of the network
	// keep some randomness to not get stuck in a loop if someones not working
	vector<shared_ptr<DHTNode>> nodes;
	vector<int> present = {id};

	for (shared_ptr<Connection>& connection : connections) {
		present.push_back(connection->id);
	}

	for (shared_ptr<DHTNode>& node : dht.nodes) {
		if (find(present.begin(), present.end(), node->id) != present.end()) continue;
		if (node->connections.size() >= 3) continue;

		nodes.push_back(node);
	}

	if (connections.size() < 2) { // 2 or 1?
		if (nodes.size() == 0) {
			for (shared_ptr<DHTNode>& node : dht.nodes) {
				if (find(present.begin(), present.end(), node->id) != present.end()) continue;
				if (node->id == dht.nodes[0]->id) continue;
				if (node->connections.size() > 3) continue;

				nodes.push_back(node);
			}
		}

		if (nodes.size() == 0) {
			for (shared_ptr<DHTNode>& node : dht.nodes) {
				if (find(present.begin(), present.end(), node->id) != present.end()) continue;
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
		int my_level = dht.getNodeFromId(id)->level;
		bool parent_found = false;

		for (int i = 0; i < connections.size() - 1; i++) {
			if (connections[i]->id == dht.nodes[0]->id) continue;

			// dont disconnect if is chess connection

			if (parent_found || dht.getNodeFromId(connections[i]->id)->level >= my_level) {
				disconnect(connections[i]->id);
				break;
			}
			else parent_found = true;
		}
	}

	if (connections.size() < 3 && !connecting) {
		int pick = pickNodeToConnect();
		if (pick == -1) return;

		connecting = true; // stupid
		cout << to_string(id) << " -> " << to_string(pick) << endl;

		if (connectToPunchholeRoot()) {
			connecting_id = pick;

			string message = "rpnp\npunchhole request " + to_string(id) + " " + to_string(pick);
			punchholeRC->writeData(message);
		}
		else {
			connecting = false;
		}
	}
}

void Node::rerootCheck() {
	is_reroot = true;

	while (dht.nodes.size() >= 2 && dht.nodes[1]->id == id) {
		Connection connection(dht.nodes[0]->ip, ROOT_PORT, dht.nodes[0]->id);
		
		if (!connection.connected) {
			vector<int> root_connections;

			for (shared_ptr<DHTConnection>& dht_connection : dht.nodes[0]->connections) {
				if (dht_connection->a->id == dht.nodes[0]->id) root_connections.push_back(dht_connection->b->id);
				else  root_connections.push_back(dht_connection->a->id);
			}

			dht.deleteNode(dht.nodes[0]->id);

			string message = "pnp\ndisconnect " + to_string(dht.nodes[0]->id);
			for (int& root_connection : root_connections) {
				if (root_connection == id) {
					disconnect(dht.nodes[0]->id);
					continue;
				}

				relay(root_connection, message);
			}

			becomeRoot();

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
		//for (shared_ptr<Connection>& connection : connections) {
		for (int i = 0; i < connections.size(); i++) {
			shared_ptr<Connection>& connection = connections[i];
			if (!connection) continue; // stupid
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

		if (connections.size() == 0 && !is_root) {
			if (!rootConnection && !connecting) {
				if (connectToRoot()) {
					if (id != -1) {
						string message = "rpnp\ndht leave " + to_string(id);
						rootConnection->writeData(message);
					}

					connect();
				}
			}
		}

		this_thread::sleep_for(chrono::milliseconds(LOOKOUT_CHECK_FREQUENCY));
	}
}

void Node::keepalive() {
	while (true) {
		string message = "keepalive";

		for (shared_ptr<Connection>& connection : connections) {
			if (connection && connection->connected) connection->writeData(message); // stupid
		}

		if (rootConnection) {
			rootConnection->writeData(message);
		}

		if (punchholeRC) {
			punchholeRC->writeData(message);
		}

		this_thread::sleep_for(chrono::seconds(KEEPALIVE_FREQUENCY));
	}
}

bool Node::connectToRoot() {
	if (rootConnection) return true;
	
	string root = getDDNS();
	rootConnection = make_shared<Connection>(root, ROOT_PORT, 0);

	if (rootConnection->connected) return true;
	else {
		rootConnection.reset();

		return false;
	}
}

bool Node::connectToPunchholeRoot() {
	if (punchholeRC) return false;

	string root = getDDNS();
	punchholeRC = make_shared<Connection>(root, ROOT_PORT, 0);

	if (punchholeRC->connected) return true;
	else {
		punchholeRC.reset();

		return false;
	}
}

void Node::punchholeConnect(string target_ip, int target_port, int target_id) {
	shared_ptr<Connection> connection = punchholeRC;
	punchholeRC.reset();

	connection->change(target_ip, target_port, target_id);
	
	if (connection->connected) {
		connections.push_back(connection);

		shared_ptr<DHTNode> a = dht.getNodeFromId(id);
		shared_ptr<DHTNode> b = dht.getNodeFromId(target_id);

		shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(id), dht.getNodeFromId(target_id));
		dht.addConnection(dht_connection);

		string message = "rpnp\ndht connect " + to_string(id) + " " + to_string(target_id);
		relay(dht.nodes[0]->id, message);
		
		cout << to_string(id) << " == " << to_string(target_id) << endl;
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
	vector<int> path;
	if (target_id == dht.nodes[0]->id) path = findPathToRoot();
	else path = findPath(target_id);
	
	if (path.size() == 0) return;

	string message = "";
	int current = id;
	int session = generateRandom(0, 999999);

	for (int i = 2; i < path.size(); i++) {
		message += "pnp\nrelay request " + to_string(path[i]) + " " + to_string(current) + " " + to_string(session) + "\n";
		current = path[i - 1];
	}

	message += payload;

	for (shared_ptr<Connection>& connection : connections) {
		if (connection->id == path[1]) {
			connection->writeData(message);
			
			break;
		}
	}

	if (path.size() > 2) {
		RelaySession relay_session = RelaySession(target_id, id, session);
		relay_sessions.push_back(relay_session);
	}
}

void Node::disconnect(int target_id) {
	string message = "pnp\ndisconnect " + to_string(id);
	shared_ptr<Connection> connection = nullptr;

	for (int i = 0; i < connections.size(); i++) {
		if (connections[i]->id == target_id) {
			connection = connections[i];
			connections[i]->writeData(message);
			connections.erase(connections.begin() + i);

			break;
		}
	}

	shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(id), dht.getNodeFromId(target_id));
	dht.deleteConnection(dht_connection);
	
	if (dht.getNodeFromId(id)->connections.size() > 0) {
		message = "rpnp\ndht disconnect " + to_string(id) + " " + to_string(target_id);
		relay(dht.nodes[0]->id, message);
	}

	if (is_root && connection) {
		int port = connection->socket->local_endpoint().port();
		root_node->port_use[port - ROOT_PORT - 1] = false;
	}

	connection.reset();
}

//static bool is_private_ip(const asio::ip::address& addr) { // also copied from openai
//	// Check for private IP address ranges
//	if (addr.is_v4()) {
//		asio::ip::address_v4::bytes_type bytes = addr.to_v4().to_bytes();
//		uint8_t firstByte = bytes[0];
//
//		// Check for common private IPv4 address ranges
//		if (firstByte == 10 || (firstByte == 192 && bytes[1] == 168) ||
//			(firstByte == 172 && (bytes[1] >= 16 && bytes[1] <= 31))) {
//			return true;
//		}
//	}
//	else if (addr.is_v6()) {
//		// Add logic to check for private IPv6 address ranges if needed
//		// This example only handles IPv4 private addresses
//	}
//
//	return false;
//}
//
//string Node::getPrivateIP() { // copied from openai, doesnt necessarily work
//	asio::io_service io;
//	asio::ip::udp::resolver resolver(io);
//	asio::ip::udp::resolver::query query(asio::ip::host_name(), "");
//	asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
//	asio::ip::udp::resolver::iterator end; // End marker.
//
//	while (iter != end) {
//		asio::ip::udp::endpoint ep = *iter++;
//		if (is_private_ip(ep.address())) {
//			std::cout << "Private IPv4 Address: " << ep.address().to_string() << std::endl;
//			return ep.address().to_string();
//		}
//	}
//
//	return "";
//}

RelaySession::RelaySession(int to, int from, int session) : to(to), from(from), session(session) {
	creation = chrono::high_resolution_clock::now();
}