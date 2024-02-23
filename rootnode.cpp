#include "node.h"
#include "connection.h"

using namespace std;

RootNode::RootNode(Node* n) : node(n) {
	admin = new OpenConnection();
	handle_thread = thread(&handleThread);

	if (node->connections.size() == 0) createDHT();
	else overtakeDHT();

	detached_thread = thread(&detachedCheck);

	cout << "i am root" << endl;
}

void RootNode::handleThread() {
	while (true) {
		if (admin->incoming_messages.size() > 0) {
			Message message = admin->incoming_messages.front();
			admin->incoming_messages.pop();

			cout << "new message from socket " << message.endpoint.address().to_string() << "::" << message.endpoint.port() << ": " << message.message << endl;
			handleMessage(message);
		}

		this_thread::yield();
	}
}

void RootNode::createDHT() {
	dht = DHT();

	DHTNode dhtNode = DHTNode(0, 0, node->getIP());

	if (dht.addNode(dhtNode)) changedDHT();
}

void RootNode::overtakeDHT() {
	dht = node->dht;
	dht.calculateLevels();
	changedDHT();
}

void RootNode::changedDHT() { // implement sending only dht changes when broadcast, add last dht save
	dht.version++;
	if (dht.version == 1000) dht.version = 1;

	string broadcast = "pnp\nbroadcast\ndht\n" + dht.toString();

	for (Connection* connection : node->connections) {
		connection->writeData(broadcast);
	}
} // dont only send changes if reroot, send whole thing with root node first

void RootNode::holepunchConnect(asio::ip::udp::endpoint a_endpoint, asio::ip::udp::endpoint b_endpoint, int a_id, int b_id) {
	string blank_message = "pnp\npunchhole info ";

	string a_message = blank_message + to_string(b_id) + " " + b_endpoint.address().to_string() + " " + to_string(b_endpoint.port());
	admin->writeData(a_endpoint, a_message);

	string b_message = blank_message + to_string(a_id) + " " + a_endpoint.address().to_string() + " " + to_string(a_endpoint.port());
	admin->writeData(b_endpoint, b_message);
}

void RootNode::detachedCheck() {
	while (true) {
		time_point now = chrono::high_resolution_clock::now();
		vector<DHTNode> bad_nodes;

		for (DHTNode dht_node : dht.nodes) {
			if (dht_node.id == node->id) continue;

			if (dht_node.disconnected) {
				if (dht_node.connections.size() > 0) {
					dht_node.disconnected = false;
				}
				else {
					int time_passed = chrono::duration_cast<chrono::seconds>(now - dht_node.disconnect_time).count();
					if (time_passed > DETACHED_DETECTION) {
						bad_nodes.push_back(dht_node);
					}
				}
			}
			else {
				if (dht_node.connections.size() == 0) {
					dht_node.disconnected = true;
					dht_node.disconnect_time = chrono::high_resolution_clock::now();
				}
			}
		}

		for (DHTNode bad_node : bad_nodes) {
			dht.deleteNode(bad_node);
		}
		changedDHT();

		this_thread::sleep_for(chrono::seconds(DETACHED_CHECK_FREQUENCY));
	}
}

void RootNode::simulateHolepunchConnect(asio::ip::udp::endpoint target_endpoint, int target_id) {
	int port = 13371 + node->connections.size(); // implement vector of used ports
	
	string message = "pnp\npunchhole info " + to_string(node->id) + " " + dht.nodes[0].ip + " " + to_string(port);
	admin->writeData(target_endpoint, message);

	Connection* connection = new Connection(target_endpoint.address().to_string(), target_endpoint.port(), target_id);
	
	if (connection->connected) {
		node->connections.push_back(connection);

		DHTConnection dht_connection = DHTConnection(dht.getNodeFromId(node->id), dht.getNodeFromId(target_id));
		if (dht.addConnection(dht_connection)) changedDHT();
	}
}
// dont forget to copy in pnp.cpp

PunchholePair::PunchholePair(int a, int b) : a(a), b(b) {}

PunchholePair::PunchholePair(int a, int b, asio::ip::udp::endpoint e) : a(a), b(b), requested_endpoint(e) {}

bool PunchholePair::operator ==(PunchholePair const& pair) {
	return a == pair.a && b == pair.b;
}