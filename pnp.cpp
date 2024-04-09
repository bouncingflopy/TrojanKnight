#include <iostream>

#include "node.h"

using namespace std;

void Node::handleMessage(shared_ptr<Connection> connection, string message) {
	vector<string> lines;
	istringstream stream(message);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	for (int i = 0; i < lines.size(); i++) {
		vector<string> words;
		istringstream stream(lines[i]);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		if (words[0] == "rpnp") {
			if (is_root) {
				root_node->handleMessage(connection, message);
			}

			break;
		}
		else if (words[0] == "cpnp") {
			if (connection->chess_connection) {
				string data;
				for (int j = i + 1; j < lines.size(); j++) {
					data += lines[j];
					if (j < lines.size() - 1) {
						data += "\n";
					}
				}

				connection->board->decodeCPNP(data);
			}

			break;
		}
		else if (words[0] == "pnp") {
			continue; // force having pnp? (and rpnp)
		}

		if (words[0] == "dht") {
			string dht_string;
			for (int j = i + 1; j < lines.size(); j++) {
				dht_string += lines[j];
				if (j < lines.size() - 1) {
					dht_string += "\n";
				}
			}

			DHT received_dht(dht_string);
			if (received_dht.version > dht.version || dht.version - received_dht.version > 990) {
				dht = received_dht;
			}

			if (!is_reroot) {
				if (reroot_thread) reroot_thread->join();
				reroot_thread = make_shared<thread>(&Node::rerootCheck, this);
			}

			rootConnection.reset();

			manageConnections();

			break;
		}
		else if (words[0] == "joined") {
			id = stoi(words[1]);
			cout << "joined as " + to_string(id) << endl;

			joined_time = make_shared<time_point>(chrono::high_resolution_clock::now());
		}
		else if (words[0] == "punchhole") {
			if (words[1] == "fail") {
				unique_lock<mutex> lock(punchholeRC_mutex);
				punchholeRC.reset();
				lock.unlock();
				connecting = false;

				manageConnections();
			}
			else if (words[1] == "invite") {
				if (connecting && connecting_id == stoi(words[2])) continue;
				
				lock_guard<mutex> lock(punchholeRC_mutex);
				if (!connectToPunchholeRoot()) {
					string message = "rpnp\npunchhole fail " + to_string(id) + " " + words[2];
					relay(dht.nodes[0]->id, message);

					continue;
				}

				string message = "rpnp\npunchhole request " + to_string(id) + " " + words[2];
				punchholeRC->writeData(message);
			}
			else if (words[1] == "info") {
				if (stoi(words[2]) == dht.nodes[0]->id) {
					thread punchhole_thread([this, words]() {
						simulatedPunchholeConnect(words[3], stoi(words[4]), stoi(words[2]));
						});
					punchhole_thread.detach();
				}
				else {
					thread punchhole_thread([this, words]() {
						punchholeConnect(words[3], stoi(words[4]), stoi(words[2]));
						});
					punchhole_thread.detach();
				}
			}
		}
		else if (words[0] == "success") {
			rootConnection.reset();
			// add dht retry
		}
		else if (words[0] == "unsuccess") {
			cout << "why is there an unsuccess, nothing should go wrong" << endl;
			rootConnection.reset();
		}
		else if (words[0] == "broadcast") {
			string broadcast;
			for (int j = i + 1; j < lines.size(); j++) {
				broadcast += lines[j];
				if (j < lines.size() - 1) {
					broadcast += "\n";
				}
			}

			handleMessage(connection, broadcast);

			string rebroadcast = "pnp\nbroadcast\n" + broadcast;
			int my_level = dht.getNodeFromId(id)->level;

			lock_guard<mutex> lock(connections_mutex);
			for (shared_ptr<Connection>& my_connection : connections) {
				if (my_level < dht.getNodeFromId(my_connection->id)->level) {
					my_connection->writeData(rebroadcast);
				}
			}

			break;
		}
		else if (words[0] == "relay") {
			if (words[1] == "request") {
				RelaySession relay_session = RelaySession(stoi(words[2]), stoi(words[3]), stoi(words[4]));
				
				string relay_message;
				for (int j = i + 1; j < lines.size(); j++) {
					relay_message += lines[j];
					if (j < lines.size() - 1) {
						relay_message += "\n";
					}
				}

				if (relay_session.to == id) {
					handleMessage(relay_session, connection, relay_message);
					break;
				}

				unique_lock<mutex> lock(connections_mutex);
				for (shared_ptr<Connection>& my_connection : connections) {
					if (my_connection->id == relay_session.to) {
						my_connection->writeData(relay_message);
						break;
					}
				}
				lock.unlock();

				relay_sessions.push_back(relay_session);

				break;
			}
			else if (words[1] == "response") {
				int session = stoi(words[2]);
				
				string relay_message;
				for (int j = i + 1; j < lines.size(); j++) {
					relay_message += lines[j];
					if (j < lines.size() - 1) {
						relay_message += "\n";
					}
				}

				for (int r = relay_sessions.size() - 1; r >= 0; r--) {
					RelaySession relay_session = relay_sessions[r];
					if (relay_session.session == session) {
						relay_sessions.erase(relay_sessions.begin() + r);

						if (relay_session.from == id) {
							handleMessage(connection, relay_message);
						}
						else {
							string response = "pnp\nrelay response " + to_string(session) + "\n" + relay_message;

							lock_guard<mutex> lock(connections_mutex);
							for (shared_ptr<Connection>& connection : connections) {
								if (connection->id == relay_session.from) {
									connection->writeData(response);
									break;
								}
							}
						}

						break;
					}
				}

				break;
			}
		}
		else if (words[0] == "disconnect") {
			disconnect(stoi(words[1]));
		}
		else if (words[0] == "chess") {
			if (words[1] == "invite") {
				incoming_invites.push_back(make_shared<ChessInvite>(id, stoi(words[2]), stoi(words[3])));
			}
			else if (words[1] == "cancel") {
				for (int i = 0; i < incoming_invites.size(); i++) {
					if (to_string(incoming_invites[i]->game) == words[2]) {
						incoming_invites.erase(incoming_invites.begin() + i);
						break;
					}
				}
			}
			else if (words[1] == "accept") {
				thread game_thread([this, words]() {
					createGame(stoi(words[2]));
					});
				game_thread.detach();
			}
			else if (words[1] == "reject") {
				if (outgoing_invite->game == stoi(words[2])) outgoing_invite.reset();
			}
			else if (words[1] == "start") {
				connection->chess_connection = true;
				int me = stoi(words[2]) % 2 ^ (id > connection->id);
				string white_player = !me ? dht.getNodeFromId(connection->id)->name : dht.getNodeFromId(id)->name;
				string black_player = me ? dht.getNodeFromId(connection->id)->name : dht.getNodeFromId(id)->name;

				connection->board = make_shared<Board>(connection, me, white_player, black_player);
				
				chess_connection = connection;
			}
		}
	}
}

void RootNode::handleMessage(Message message) {
	vector<string> lines;
	istringstream stream(message.message);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	for (string line : lines) {
		vector<string> words;
		istringstream stream(line);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		if (words[0] == "rpnp") {
			continue;
		}

		if (words[0] == "dht") {
			if (words[1] == "request") {
				string response = "pnp\ndht\n" + dht.toString();
				admin->writeData(message.endpoint, response);
			}
			else if (words[1] == "join") {
				shared_ptr<DHTNode> node = make_shared<DHTNode>();
				node->id = dht.getFreeId();
				node->level = -1;
				node->ip = message.endpoint.address().to_string();
				node->name = to_string(node->id);

				if (dht.addNode(node)) changedDHT();

				string response = "pnp\njoined " + to_string(node->id);
				admin->writeData(message.endpoint, response);
			}
			else if (words[1] == "connect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				shared_ptr<DHTConnection> connection = make_shared<DHTConnection>(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (connection->a->id == -1 || connection->b->id == -1) {
					string response = "pnp\nunsuccess";
					admin->writeData(message.endpoint, response);

					continue;
				}

				if (dht.addConnection(connection)) changedDHT();

				string response = "pnp\nsuccess";
				admin->writeData(message.endpoint, response);
			}
			else if (words[1] == "disconnect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				shared_ptr<DHTConnection> connection = make_shared<DHTConnection>(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (connection->a->id == -1 || connection->b->id == -1) {
					string response = "pnp\nunsuccess";
					admin->writeData(message.endpoint, response);

					continue;
				}

				if (dht.deleteConnection(connection)) changedDHT();

				string response = "pnp\nsuccess";
				admin->writeData(message.endpoint, response);
			}
			else if (words[1] == "leave") {
				if (dht.deleteNode(stoi(words[2]))) changedDHT();
			}
		}
		else if (words[0] == "punchhole") {
			if (words[1] == "fail") {
				PunchholePair failed_pair = PunchholePair(stoi(words[2]), stoi(words[3]));
				asio::ip::udp::endpoint requested_endpoint;

				bool found = false;
				lock_guard<mutex> lock(punchhole_pairs_mutex);
				for (int i = 0; i < punchhole_pairs.size(); i++) {
					if (failed_pair == punchhole_pairs[i]) {
						requested_endpoint = punchhole_pairs[i].requested_endpoint;
						punchhole_pairs.erase(punchhole_pairs.begin() + i);
						found = true;
						break;
					}
				}

				if (found) {
					string report = "pnp\npunchhole fail";
					admin->writeData(requested_endpoint, report);
				}
			}
			else if (words[1] == "request") {
				PunchholePair pair = PunchholePair(stoi(words[2]), stoi(words[3]), message.endpoint);

				if (pair.b == node->id) {
					if (node->connections.size() < 3) {
						thread punchhole_thread([this, pair]() {
							simulateHolepunchConnect(pair.requested_endpoint, pair.a);
						});
						punchhole_thread.detach();
						continue;
					}
				}

				bool waiting = false;
				int i;
				lock_guard<mutex> lock(punchhole_pairs_mutex);
				for (i = 0; i < punchhole_pairs.size(); i++) {
					if (pair == punchhole_pairs[i]) {
						waiting = true;
						break;
					}
				}

				if (!waiting) {
					punchhole_pairs.push_back(pair);

					string invite = "pnp\npunchhole invite " + words[2];
					node->relay(stoi(words[3]), invite);
				}
				else {
					if (pair.requested_endpoint != punchhole_pairs[i].requested_endpoint) {
						holepunchConnect(pair.requested_endpoint, punchhole_pairs[i].requested_endpoint, pair.a, pair.b);
						punchhole_pairs.erase(remove(punchhole_pairs.begin(), punchhole_pairs.end(), pair), punchhole_pairs.end());
					}
				}
			}
		}
	}
}

// update by copy from other handleMessage

void Node::handleMessage(RelaySession relay_session, shared_ptr<Connection> connection, string message) {
	vector<string> lines;
	istringstream stream(message);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	for (int i = 0; i < lines.size(); i++) {
		vector<string> words;
		istringstream stream(lines[i]);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		if (words[0] == "rpnp") {
			if (is_root) {
				root_node->handleMessage(relay_session, connection, message);
			}

			break;
		}
		else {
			handleMessage(connection, message);
		}
	}
}

void Node::handleMessage(string message) {
	vector<string> lines;
	istringstream stream(message);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	for (int i = 0; i < lines.size(); i++) {
		vector<string> words;
		istringstream stream(lines[i]);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		if (words[0] == "pnp") {
			continue; // force having pnp? (and rpnp)
		}

		if (words[0] == "dht") {
			string dht_string;
			for (int j = i + 1; j < lines.size(); j++) {
				dht_string += lines[j];
				if (j < lines.size() - 1) {
					dht_string += "\n";
				}
			}

			DHT received_dht(dht_string);
			if (received_dht.version > dht.version || dht.version - received_dht.version > 990) {
				dht = received_dht;
			}

			break;
		}
		else if (words[0] == "broadcast") {
			string broadcast;
			for (int j = i + 1; j < lines.size(); j++) {
				broadcast += lines[j];
				if (j < lines.size() - 1) {
					broadcast += "\n";
				}
			}

			handleMessage(broadcast);

			string rebroadcast = "pnp\nbroadcast\n" + broadcast;

			lock_guard<mutex> lock(connections_mutex);
			for (shared_ptr<Connection>& my_connection : connections) {
				if (my_connection->connected) my_connection->writeData(rebroadcast);
			}

			break;
		}
	}
}

void RootNode::handleMessage(string message) {
	vector<string> lines;
	istringstream stream(message);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	for (string line : lines) {
		vector<string> words;
		istringstream stream(line);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		if (words[0] == "rpnp") {
			continue;
		}

		if (words[0] == "dht") {
			if (words[1] == "connect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				shared_ptr<DHTConnection> connection = make_shared<DHTConnection>(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (connection->a->id == -1 || connection->b->id == -1) continue; // no feedback

				if (dht.addConnection(connection)) changedDHT();
			}
			else if (words[1] == "disconnect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				shared_ptr<DHTConnection> connection = make_shared<DHTConnection>(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (connection->a->id == -1 || connection->b->id == -1)  continue; // no feedback

				if (dht.deleteConnection(connection)) changedDHT();
			}
			else if (words[1] == "rename") {
				if (dht.renameNode(stoi(words[2]), words[3])) changedDHT();
			}
		}
	}
}

void RootNode::handleMessage(shared_ptr<Connection> connection, string message) {
	vector<string> lines;
	istringstream stream(message);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	for (string line : lines) {
		vector<string> words;
		istringstream stream(line);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		if (words[0] == "rpnp") {
			continue;
		}

		if (words[0] == "dht") {
			if (words[1] == "request") {
				string response = "pnp\n" + dht.toString();
				connection->writeData(response);
			}
			else if (words[1] == "connect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (dht_connection->a->id == -1 || dht_connection->b->id == -1) {
					string response = "pnp\nunsuccess";
					connection->writeData(response);

					continue;
				}

				if (dht.addConnection(dht_connection)) changedDHT();

				string response = "pnp\nsuccess";
				connection->writeData(response);
			}
			else if (words[1] == "disconnect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (dht_connection->a->id == -1 || dht_connection->b->id == -1) {
					string response = "pnp\nunsuccess";
					connection->writeData(response);

					continue;
				}

				if (dht.deleteConnection(dht_connection)) changedDHT();

				string response = "pnp\nsuccess";
				connection->writeData(response);
			}
			else if (words[1] == "leave") {
				if (dht.deleteNode(stoi(words[2]))) changedDHT();
			}
			else if (words[1] == "rename") {
				if (dht.renameNode(stoi(words[2]), words[3])) changedDHT();
			}
		}
		else if (words[0] == "punchhole") {
			if (words[1] == "fail") {
				PunchholePair failed_pair = PunchholePair(stoi(words[2]), stoi(words[3]));
				asio::ip::udp::endpoint requested_endpoint;

				bool found = false;
				lock_guard<mutex> lock(punchhole_pairs_mutex);
				for (int i = 0; i < punchhole_pairs.size(); i++) {
					if (failed_pair == punchhole_pairs[i]) {
						requested_endpoint = punchhole_pairs[i].requested_endpoint;
						punchhole_pairs.erase(punchhole_pairs.begin() + i);
						found = true;
						break;
					}
				}

				if (found) {
					string report = "pnp\npunchhole fail";
					admin->writeData(requested_endpoint, report);
				}
			}
		}
	}
}

void RootNode::handleMessage(RelaySession relay_session, shared_ptr<Connection> connection, string message) {
	string relay_response = "pnp\nrelay response " + to_string(relay_session.session) + "\n";
	
	vector<string> lines;
	istringstream stream(message);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	for (string line : lines) {
		vector<string> words;
		istringstream stream(line);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		if (words[0] == "rpnp") {
			continue;
		}

		if (words[0] == "dht") {
			if (words[1] == "request") {
				string response = "pnp\n" + dht.toString();
				connection->writeData(relay_response + response);
			}
			else if (words[1] == "connect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (dht_connection->a->id == -1 || dht_connection->b->id == -1) {
					string response = "pnp\nunsuccess";
					connection->writeData(relay_response + response);

					continue;
				}

				if (dht.addConnection(dht_connection)) changedDHT();

				string response = "pnp\nsuccess";
				connection->writeData(relay_response + response);
			}
			else if (words[1] == "disconnect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (dht_connection->a->id == -1 || dht_connection->b->id == -1) {
					string response = "pnp\nunsuccess";
					connection->writeData(relay_response + response);

					continue;
				}

				if (dht.deleteConnection(dht_connection)) changedDHT();

				string response = "pnp\nsuccess";
				connection->writeData(relay_response + response);
			}
			else if (words[1] == "leave") {
				if (dht.deleteNode(stoi(words[2]))) changedDHT();
			}
			else if (words[1] == "rename") {
				if (dht.renameNode(stoi(words[2]), words[3])) changedDHT();
			}
		}
		else if (words[0] == "punchhole") {
			if (words[1] == "fail") {
				PunchholePair failed_pair = PunchholePair(stoi(words[2]), stoi(words[3]));
				asio::ip::udp::endpoint requested_endpoint;

				bool found = false;
				lock_guard<mutex> lock(punchhole_pairs_mutex);
				for (int i = 0; i < punchhole_pairs.size(); i++) {
					if (failed_pair == punchhole_pairs[i]) {
						requested_endpoint = punchhole_pairs[i].requested_endpoint;
						punchhole_pairs.erase(punchhole_pairs.begin() + i);
						found = true;
						break;
					}
				}

				if (found) {
					string report = "pnp\npunchhole fail";
					admin->writeData(requested_endpoint, report);
				}
			}
		}
	}
}