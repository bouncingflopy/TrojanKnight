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
	vector<DHTNode*> nodes;
	vector<DHTConnection*> connections;

	DHT();
	DHT(string dht);
	string toString();
	bool addNode(DHTNode* node);
	bool addConnection(DHTConnection* connection);
	bool deleteNode(int id);
	bool deleteConnection(DHTConnection* connection); // needed?
	DHTNode* getNodeFromId(int id);
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
	vector<DHTConnection*> connections;
	bool disconnected = false;
	time_point disconnect_time;

	DHTNode();
	DHTNode(int id, int level, string ip);
	bool operator ==(DHTNode const& node);
};

struct DHTConnection {
	DHTNode* a;
	DHTNode* b;

	DHTConnection();
	DHTConnection(DHTNode* a, DHTNode* b);
	bool operator ==(DHTConnection const& connection);
};

#endif