#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>

#include "connection.h"
#include "networksettings.h"
#include "dht.h"

class Connection;
class OpenConnection;
class RootNode;
struct DHT;
struct Message;
struct RelaySession;
struct PunchholePair;

class Node {
public:
	int id = -1;
	vector<Connection*> connections;
	Connection* rootConnection = nullptr;
	bool is_root = false;
	bool is_reroot = false;
	bool connecting = false;
	int connecting_id = -1;
	RootNode* root_node;
	thread handle_thread;
	thread keepalive_thread;
	thread lookout_thread;
	thread* reroot_thread = nullptr;
	DHT dht;
	vector<RelaySession> relay_sessions;

	Node();
	string getIP();
	//string getPrivateIP();
	string getDDNS();
	void setDDNS(string ip);
	void becomeRoot();
	void connect();
	void handleThread();
	void handleMessage(Connection* connection, string message);
	void handleMessage(RelaySession relay_session, Connection* connection, string message);
	void handleMessage(string message);
	int pickNodeToConnect();
	void manageConnections();
	void rerootCheck();
	void lookout();
	void keepalive();
	bool connectToRoot();
	void punchholeConnect(string target_ip, int target_port, int target_id);
	vector<int> findPathToRoot();
	vector<int> findPath(int id);
	void relay(int target_id, string payload);
	void disconnect(int id);
};

struct RelaySession {
	int to;
	int from;
	int session;
	time_point creation;

	RelaySession(int to, int from, int session);
};

// add and implement private ip connection, only query written
// what if node sends root dht update (connect and disconnect) but root is rerooting or doesnt get it?
// root check no one takes ddns, if im empty connect to them (mid reroot)

/*

rpnp
v	dht request
v	dht join
v	dht connect 0 1
v	dht disconnect 0 1
v	dht leave 1
v	punchhole request 0 1
v	punchhole fail 0 1

	private response 1 192.168.1.1

*/

/*

pnp
v	dht
v		...
v	joined 1
v	punchhole invite [by] 1
v	punchhole info 1 1.2.3.4 1337
v	punchhole fail
v	success
v	unsuccess
v	broadcast
v		...
v	relay request [to] 1 [from] 2 [session] 1
v		...
v	relay response [session] 1
v		...
v	disconnect 1

	private query

*/

/*

keepalive
syn
ack

*/

// delete all pointers when closing program
// make chess board size based on computer size
// endcryption public key private key
// detach thread?

class RootNode {
public:
	Node* node;
	OpenConnection* admin;
	thread handle_thread;
	thread detached_thread;
	DHT dht;
	vector<PunchholePair> punchhole_pairs;
	vector<bool> port_use = {false, false, false};

	RootNode(Node* n);
	void createDHT();
	void overtakeDHT();
	void handleThread();
	void handleMessage(Message message);
	void handleMessage(Connection* connection, string message);
	void handleMessage(RelaySession relay_session, Connection* connection, string message);
	void changedDHT();
	void holepunchConnect(asio::ip::udp::endpoint a_endpoint, asio::ip::udp::endpoint b_endpoint, int a_id, int b_id);
	void detachedCheck();
	void simulateHolepunchConnect(asio::ip::udp::endpoint target_endpoint, int target_id);
};

struct PunchholePair {
	int a;
	int b;
	asio::ip::udp::endpoint requested_endpoint;

	PunchholePair(int a, int b);
	PunchholePair(int a, int b, asio::ip::udp::endpoint e);
	bool operator ==(PunchholePair const& pair);
};

#endif