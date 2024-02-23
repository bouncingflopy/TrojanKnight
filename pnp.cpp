#include <iostream>

#include "node.h"

using namespace std;

void Node::handleMessage(Connection* connection, string message) {
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
			continue; // add connection to board class and boardcpnp
		}
		else if (words[0] == "pnp") {
			continue; // force having pnp? (and rpnp)
		}

		if (words[0] == "dht") {
			string dht_string = "";
			for (int j = i + 1; j < lines.size(); j++) {
				dht_string += lines[j];
				if (j < lines.size() - 1) {
					dht_string += "\n";
				}
			}

			DHT received_dht = DHT(dht_string);
			if (received_dht.version > dht.version || dht.version - received_dht.version > 990) {
				dht = received_dht;
			}

			if (!is_reroot) reroot_thread = thread(&rerootCheck);

			break;
		}
		else if (words[0] == "joined") {
			id = stoi(words[1]);

			manageConnections();
		}
		else if (words[0] == "punchhole") {
			if (words[1] == "fail") {
				delete rootConnection;
				rootConnection = nullptr;

				manageConnections();
			}
			else if (words[1] == "invite") {
				if (rootConnection) {
					string message = "rpnp\npunchhole fail " + words[2] + " " + to_string(id);
					rootConnection->writeData(message);

					continue;
				}
				else if (!connectToRoot()) {
					string message = "rpnp\npunchhole fail " + words[2] + " " + to_string(id);
					relay(dht.nodes[0].id, message);

					continue;
				}

				string message = "rpnp\npunchhole " + words[2] + " " + to_string(id);
				rootConnection->writeData(message);
			}
			else if (words[1] == "info") {
				punchholeConnect(words[3], stoi(words[4]), stoi(words[2]));
				manageConnections();
			}
		}
		else if (words[0] == "success") {
			if (rootConnection) {
				delete rootConnection;
				rootConnection = nullptr;
			}
		}
		else if (words[0] == "unsuccess") {
			cout << "why is there an unsuccess, nothing should go wrong" << endl;
			if (rootConnection) {
				delete rootConnection;
				rootConnection = nullptr;
			}
		}
		else if (words[0] == "broadcast") {
			string broadcast = "";
			for (int j = i + 1; j < lines.size(); j++) {
				broadcast += lines[j];
				if (j < lines.size() - 1) {
					broadcast += "\n";
				}
			}

			handleMessage(connection, broadcast);

			string rebroadcast = "pnp\nbroadcast\n" + broadcast;
			int my_level = dht.getNodeFromId(id)->level;

			for (Connection* connection : connections) {
				if (my_level > dht.getNodeFromId(connection->id)->level) {
					connection->writeData(rebroadcast);
				}
			}

			break;
		}
		else if (words[0] == "relay") {
			if (words[1] == "request") {
				RelaySession relay_session = RelaySession(stoi(words[2]), stoi(words[3]), stoi(words[4]));
				
				string relay_message = "";
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

				for (Connection* connection : connections) {
					if (connection->id == relay_session.to) {
						connection->writeData(relay_message);
						break;
					}
				}

				relay_sessions.push_back(relay_session);

				break;
			}
			else if (words[1] == "response") {
				int session = stoi(words[2]);
				
				string relay_message = "";
				for (int j = i + 1; j < lines.size(); j++) {
					relay_message += lines[j];
					if (j < lines.size() - 1) {
						relay_message += "\n";
					}
				}

				for (int r = relay_sessions.size() - 1; r >= 0; r--) {
					RelaySession relay = relay_sessions[r];
					if (relay.session == session) {
						relay_sessions.erase(relay_sessions.begin() + r);

						if (relay.from == id) {
							handleMessage(connection, relay_message);
						}
						else {
							string response = "pnp\nrelay response " + to_string(session) + "\n" + relay_message;
						}

						break;
					}
				}
			}
		}
		else if (words[0] == "disconnect") {
			disconnect(stoi(words[1]));
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
				string response = "pnp\n" + dht.toString();
				admin->writeData(message.endpoint, response);
			}
			else if (words[1] == "join") {
				DHTNode node = DHTNode();
				node.id = dht.getFreeId();
				node.level = -1;
				node.ip = message.endpoint.address().to_string();

				if (dht.addNode(node)) changedDHT();

				string response = "pnp";
				response += "\njoined " + node.id;
				admin->writeData(message.endpoint, response);
			}
			else if (words[1] == "connect") {
				int a = stoi(words[2]);
				int b = stoi(words[3]);
				DHTConnection connection = DHTConnection(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (connection.a->id == -1 || connection.b->id == -1) {
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
				DHTConnection connection = DHTConnection(dht.getNodeFromId(a), dht.getNodeFromId(b));

				if (connection.a->id == -1 || connection.b->id == -1) {
					string response = "pnp\nunsuccess";
					admin->writeData(message.endpoint, response);

					continue;
				}

				if (dht.deleteConnection(connection)) changedDHT();

				string response = "pnp\nsuccess";
				admin->writeData(message.endpoint, response);
			}
			else if (words[1] == "leave") {
				DHTNode node = DHTNode();
				node.id = stoi(words[2]);

				if (dht.deleteNode(node)) changedDHT();
			}
		}
		else if (words[0] == "punchhole") {
			if (words[1] == "fail") {
				PunchholePair failed_pair = PunchholePair(stoi(words[2]), stoi(words[3]));
				asio::ip::udp::endpoint requested_endpoint;

				for (int i = 0; i < punchhole_pairs.size(); i++) {
					if (failed_pair == punchhole_pairs[i]) {
						requested_endpoint = punchhole_pairs[i].requested_endpoint;
						punchhole_pairs.erase(punchhole_pairs.begin() + i);
						break;
					}
				}

				string report = "pnp\npunchhole fail";
				admin->writeData(requested_endpoint, report);
			}
			else if (words[1] == "request") {
				PunchholePair pair = PunchholePair(stoi(words[2]), stoi(words[3]), message.endpoint);

				if (pair.b == node->id) {
					simulateHolepunchConnect(pair.requested_endpoint, pair.a);
					continue;
				}

				bool waiting = false;
				for (PunchholePair other_pair : punchhole_pairs) {
					if (pair == other_pair) waiting = true;
				}

				if (!waiting) {
					punchhole_pairs.push_back(pair);

					string invite = "pnp\npunchhole invite " + words[2];
					node->relay(stoi(words[3]), invite);
				}
				else {
					if (pair.requested_endpoint != message.endpoint) {
						holepunchConnect(pair.requested_endpoint, message.endpoint, pair.a, pair.b);
					}
				}
			}
		}
	}
}

// update by copy from other handleMessage
/*



































*/

// make have proper relay response
void Node::handleMessage(RelaySession relay, Connection* connection, string message) {}

void RootNode::handleMessage(Connection* connection, string message) {}

void RootNode::handleMessage(RelaySession relay, Connection* connection, string message) {}