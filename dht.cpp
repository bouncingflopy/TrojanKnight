#include <string>
#include <sstream>
#include <algorithm>
#include <queue>

#include "dht.h"

DHT::DHT() : version(0) {}

DHT::DHT(string dht) {
	vector<string> lines;
	istringstream stream(dht);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	version = stoi(lines[0]);

	int i;
	for (i = 2; lines[i] != "-"; i++) {
		vector<string> words;
		istringstream stream(lines[i]);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		shared_ptr<DHTNode> node = make_shared<DHTNode>(stoi(words[0]), stoi(words[1]), words[2]);
		nodes.push_back(node);
	}

	for (i++; i < lines.size(); i++) {
		vector<string> words;
		istringstream stream(lines[i]);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		shared_ptr<DHTNode> a = getNodeFromId(stoi(words[0]));
		shared_ptr<DHTNode> b = getNodeFromId(stoi(words[1]));

		shared_ptr<DHTConnection> connection = make_shared<DHTConnection>(a, b);
		connections.push_back(connection);
		
		a->connections.push_back(connection);
		b->connections.push_back(connection);
	}
}

string DHT::toString() {
	string dht = "";
	
	dht += to_string(version) + "\n-\n";

	//for (shared_ptr<DHTNode>& node : nodes) {
	//for (int i = 0; i < nodes.size(); i++) {
	//	shared_ptr<DHTNode> node = nodes[i];
	//	if (!node) continue; // stupid 1
	vector<shared_ptr<DHTNode>> copied_nodes;
	for (shared_ptr<DHTNode> node : nodes) {
		if (node) copied_nodes.push_back(shared_ptr<DHTNode>(node));
	}
	for (shared_ptr<DHTNode>& node : copied_nodes) {
		dht += to_string(node->id) + " " + to_string(node->level) + " " + node->ip + "\n";
	}
	dht += "-\n";

	//for (shared_ptr<DHTConnection>& connection : connections) {
	//for (int i = 0; i < connections.size(); i++) {
	//	shared_ptr<DHTConnection> connection = connections[i];
	//	if (!connection) continue; // stupid 1
	vector<shared_ptr<DHTConnection>> copied_connections;
	for (shared_ptr<DHTConnection> connection : connections) {
		if (connection) copied_connections.push_back(shared_ptr<DHTConnection>(connection));
	}
	for (shared_ptr<DHTConnection>& connection : copied_connections) {
		dht += to_string(connection->a->id) + " " + to_string(connection->b->id) + "\n";
	}

	return dht;
}

bool DHT::addNode(shared_ptr<DHTNode> node) {
	if (node->id == -1) return false;

	for (int i = 0; i < nodes.size(); i++) {
		if (*node == *nodes[i]) {
			return false;
		}
	}

	nodes.push_back(node);
	return true;
}

bool DHT::addConnection(shared_ptr<DHTConnection> connection) {
	if (connection->a->id == -1 || connection->b->id == -1) return false;

	for (int i = 0; i < connections.size(); i++) {
		if (*connection == *connections[i]) {
			return false;
		}
	}

	connections.push_back(connection);

	connection->a->connections.push_back(connection);
	connection->b->connections.push_back(connection);

	calculateLevels();

	return true;
}

bool DHT::deleteNode(int id) {
	if (id == -1) return false;

	for (int i = 0; i < nodes.size(); i++) {
		if (id == nodes[i]->id) {
			vector<shared_ptr<DHTConnection>> copied_connections = nodes[i]->connections;
			for (shared_ptr<DHTConnection> connection : copied_connections) {
				if (connection) deleteConnection(connection);
			}

			nodes.erase(nodes.begin() + i);

			return true;
		}
	}

	return false;
}

bool DHT::deleteConnection(shared_ptr<DHTConnection> connection) {
	if (connection->a->id == -1 || connection->b->id == -1) return false;

	for (int i = 0; i < connections.size(); i++) {
		if (*connection == *connections[i]) {
			vector<shared_ptr<DHTConnection>>& a_connections = connections[i]->a->connections;
			vector<shared_ptr<DHTConnection>>& b_connections = connections[i]->b->connections;

			a_connections.erase(remove(a_connections.begin(), a_connections.end(), connections[i]), a_connections.end());
			b_connections.erase(remove(b_connections.begin(), b_connections.end(), connections[i]), b_connections.end());

			connections.erase(connections.begin() + i);

			calculateLevels();

			return true;
		}
	}

	return false;
}

shared_ptr<DHTNode> DHT::getNodeFromId(int id) {
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == id) {
			return nodes[i];
		}
	}
	
	shared_ptr<DHTNode> empty = make_shared<DHTNode>();
	empty->id = -1;

	return empty;
}

int DHT::getFreeId() {
	vector<int> ids;
	for (shared_ptr<DHTNode>& node : nodes) {
		ids.push_back(node->id);
	}

	sort(ids.begin(), ids.end());

	int available = 0;
	for (int id : ids) {
		if (id == available) {
			available++;
		}
		else if (id > available) {
			return available;
		}
	}

	return available;
}

void DHT::calculateLevels() {
	for (shared_ptr<DHTNode>& node : nodes) {
		node->level = -1;
	}

	queue<shared_ptr<DHTNode>> current_queue;
	queue<shared_ptr<DHTNode>> next_queue;
	int level = 0;
	current_queue.push(nodes[0]);

	while (!current_queue.empty()) {
		while (!current_queue.empty()) {
			shared_ptr<DHTNode> current_node = current_queue.front();
			current_queue.pop();

			if (current_node->level != -1) continue;

			current_node->level = level;

			for (shared_ptr<DHTConnection>& connection : current_node->connections) {
				if (connection->a->level == -1) next_queue.push(connection->a);
				if (connection->b->level == -1) next_queue.push(connection->b);
			}
		}

		if (current_queue.empty()) {
			level++;

			while (!next_queue.empty()) {
				current_queue.push(next_queue.front());
				next_queue.pop();
			}
		}
	}

	current_queue = queue<shared_ptr<DHTNode>>();
	next_queue = queue<shared_ptr<DHTNode>>();
}

DHTNode::DHTNode() {};

DHTNode::DHTNode(int id, int level, string ip) : id(id), level(level), ip(ip) {};

bool DHTNode::operator ==(DHTNode const& node) {
	return id == node.id;
}

DHTConnection::DHTConnection() {};

DHTConnection::DHTConnection(shared_ptr<DHTNode> a, shared_ptr<DHTNode> b) : a(a), b(b) {};

bool DHTConnection::operator ==(DHTConnection const& connection) {
	return (a == connection.a && b == connection.b) || (a == connection.b && b == connection.a);
}