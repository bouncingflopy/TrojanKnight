#ifndef DHT_H
#define DHT_H

#include <string>
#include <vector>

#include "networksettings.h"

using namespace std;

struct DHTNode;
struct DHTConnection;

struct DHT {
	int version;
	vector<shared_ptr<DHTNode>> nodes;
	vector<shared_ptr<DHTConnection>> connections;

	DHT();
	DHT(string dht);
	string toString();
	bool addNode(shared_ptr<DHTNode> node);
	bool addConnection(shared_ptr<DHTConnection> connection);
	bool deleteNode(int id);
	bool deleteConnection(shared_ptr<DHTConnection> connection); // needed?
	shared_ptr<DHTNode> getNodeFromId(int id);
	int getFreeId();
	void calculateLevels();
};

// add names for chess

/*

version
-
nodes
id	level	ip
0	0		1.2.3.4
-
connections
id	id
0	1
0	2
1	2

*/

struct DHTNode {
	int id;
	int level;
	string ip;
	vector<shared_ptr<DHTConnection>> connections;
	bool disconnected = false;
	time_point disconnect_time;

	DHTNode();
	DHTNode(int id, int level, string ip);
	bool operator ==(DHTNode const& node);
};

struct DHTConnection {
	shared_ptr<DHTNode> a;
	shared_ptr<DHTNode> b;

	DHTConnection();
	DHTConnection(shared_ptr<DHTNode> a, shared_ptr<DHTNode> b);
	bool operator ==(DHTConnection const& connection);
};

#endif