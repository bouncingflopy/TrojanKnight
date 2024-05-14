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
	mutex nodes_mutex;
	mutex connections_mutex;

	DHT();
	DHT(string dht);
	string toString();
	bool addNode(shared_ptr<DHTNode> node);
	bool addConnection(shared_ptr<DHTConnection> connection);
	bool deleteNode(int id);
	bool deleteConnection(shared_ptr<DHTConnection> connection);
	shared_ptr<DHTNode> getNodeFromId(int id);
	shared_ptr<DHTNode> getNodeFromName(string name);
	int getFreeId();
	bool checkNameFree(string name);
	bool renameNode(int id, string name);
	void calculateLevels();
	void copyConnections(vector<shared_ptr<DHTConnection>>& copy);
	void copyNodes(vector<shared_ptr<DHTNode>>& copy);
	DHT& operator =(const DHT& other);
	bool operator ==(DHT& other);
};

// add names for chess

/*

version
-
nodes
id	level	ip			name
0	0		1.2.3.4		username
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
	string name;
	vector<shared_ptr<DHTConnection>> connections;
	bool disconnected = false;
	time_point disconnect_time;

	DHTNode();
	DHTNode(int id, int level, string ip);
	DHTNode(int id, int level, string ip, string name);
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