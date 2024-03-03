#include "node.h"
#include "connection.h"

using namespace std;

RootNode::RootNode() {}

RootNode::RootNode(shared_ptr<Node> n) : node(n) {
	admin = make_shared<OpenConnection>();
	handle_thread = thread(&RootNode::handleThread, this);

	if (node->dht.version == 0) createDHT();
	else overtakeDHT();

	detached_thread = thread(&RootNode::detachedCheck, this);

	cout << "i am (g)root" << endl;
}

void RootNode::handleThread() {
	while (!left) { // lan
		if (admin->incoming_messages.size() > 0) {
			Message message = admin->incoming_messages.front();
			admin->incoming_messages.pop();

			handleMessage(message);
		}

		this_thread::sleep_for(chrono::milliseconds(HANDLE_FREQUENCY));
	}
}

void RootNode::createDHT() {
	dht = DHT();

	shared_ptr<DHTNode> dhtNode = make_shared<DHTNode>(0, 0, node->getIP());

	if (dht.addNode(dhtNode)) changedDHT();

	node->id = 0;
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
	node->handleMessage(broadcast);
} // dont only send changes if reroot, send whole thing with root node first

void RootNode::holepunchConnect(asio::ip::udp::endpoint a_endpoint, asio::ip::udp::endpoint b_endpoint, int a_id, int b_id) {
	string blank_message = "pnp\npunchhole info ";

	string a_message = blank_message + to_string(b_id) + " " + b_endpoint.address().to_string() + " " + to_string(b_endpoint.port());
	admin->writeData(a_endpoint, a_message);

	string b_message = blank_message + to_string(a_id) + " " + a_endpoint.address().to_string() + " " + to_string(a_endpoint.port());
	admin->writeData(b_endpoint, b_message);}

void RootNode::detachedCheck() {
	while (!left) { // lan
		time_point now = chrono::high_resolution_clock::now();
		vector<int> bad_nodes;

		for (shared_ptr<DHTNode>& dht_node : dht.nodes) {
			if (dht_node->id == node->id) continue;

			if (dht_node->disconnected) {
				if (dht_node->level != -1) {
					dht_node->disconnected = false;
				}
				else {
					int time_passed = chrono::duration_cast<chrono::seconds>(now - dht_node->disconnect_time).count();
					if (time_passed > DETACHED_DETECTION) {
						bad_nodes.push_back(dht_node->id);
					}
				}
			}
			else {
				if (dht_node->level == -1) {
					dht_node->disconnected = true;
					dht_node->disconnect_time = chrono::high_resolution_clock::now();
				}
			}
		}

		if (bad_nodes.size() > 0) {
			for (int& bad_node : bad_nodes) {
				dht.deleteNode(bad_node);
			}
			changedDHT();
		}

		this_thread::sleep_for(chrono::seconds(DETACHED_CHECK_FREQUENCY));
	}
}

void RootNode::simulateHolepunchConnect(asio::ip::udp::endpoint target_endpoint, int target_id) {
	int port = 0;
	for (int i = 0; i < port_use.size(); i++) {
		if (!port_use[i]) {
			port = ROOT_PORT + i + 1;
			port_use[i] = true;
			break;
		}
	}
	if (!port) {
		return;
	}

	string message = "pnp\npunchhole info " + to_string(node->id) + " " + dht.nodes[0]->ip + " " + to_string(port);
	admin->writeData(target_endpoint, message);

	shared_ptr<Connection> connection = make_shared<Connection>(target_endpoint.address().to_string(), target_endpoint.port(), target_id, port);
	
	if (connection->connected) {
		unique_lock<mutex> lock(node->connections_mutex);
		node->connections.push_back(connection);
		lock.unlock();

		shared_ptr<DHTConnection> dht_connection = make_shared<DHTConnection>(dht.getNodeFromId(node->id), dht.getNodeFromId(target_id));
		if (dht.addConnection(dht_connection)) changedDHT();

		cout << to_string(node->id) << " == " + to_string(target_id) << endl;
	}
	else {
		port_use[port - ROOT_PORT - 1] = false;
	}
}

void RootNode::leave() {
	admin->socket->close();
	left = true;
}

PunchholePair::PunchholePair(int a, int b) : a(a), b(b) {}

PunchholePair::PunchholePair(int a, int b, asio::ip::udp::endpoint e) : a(a), b(b), requested_endpoint(e) {}

bool PunchholePair::operator ==(PunchholePair const& pair) {
	return (a == pair.a && b == pair.b) || (a == pair.b && b == pair.a);
}