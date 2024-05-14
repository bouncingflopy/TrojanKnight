#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>
#include <openssl/rsa.h>
#include <queue>

#include "connection.h"
#include "networksettings.h"
#include "dht.h"
#include "encryption.h"
#include "storage.h"
#include "console.h"

class Connection;
class OpenConnection;
class RootNode;
struct DHT;
struct Message;
struct RelaySession;
struct PunchholePair;
class Console;

class Node {
public:
	int id = -1;
	string ip;
	vector<shared_ptr<Connection>> connections;
	shared_ptr<Connection> rootConnection;
	shared_ptr<Connection> punchholeRC;
	time_point punchholeRC_creation;
	bool is_root = false;
	bool is_reroot = false;
	bool connecting = false;
	bool in_network = false;
	int connecting_id = -1;
	shared_ptr<RootNode> root_node;
	thread handle_thread;
	thread keepalive_thread;
	thread lookout_thread;
	thread name_thread;
	shared_ptr<thread> reroot_thread;
	DHT dht;
	vector<RelaySession> relay_sessions;
	mutex punchholeRC_mutex;
	mutex connections_mutex;
	shared_ptr<time_point> joined_time;
	shared_ptr<RSA> public_key;
	shared_ptr<RSA> private_key;
	shared_ptr<Console> console;

	Node();
	~Node();
	string getIP();
	string getDDNS();
	void setDDNS(string ip);
	void becomeRoot();
	void connect();
	void handleThread();
	void handleMessage(shared_ptr<Connection> connection, string message);
	void handleMessage(RelaySession relay_session, shared_ptr<Connection> connection, string message);
	void handleMessage(string message);
	int pickNodeToConnect();
	void manageConnections();
	void rerootCheck();
	void lookout();
	void keepalive();
	bool connectToRoot();
	bool connectToPunchholeRoot();
	void punchholeConnect(string target_ip, int target_port, int target_id);
	void simulatedPunchholeConnect(string target_ip, int target_port, int target_id);
	vector<int> findPathToRoot();
	vector<int> findPath(int id);
	void relay(int target_id, string payload);
	void disconnect(int target_id);
	void copyConnections(vector<shared_ptr<Connection>>& copy);
	bool checkConnectedToNode(int target_id);
	shared_ptr<Connection> getConnectionToNode(int target_id);
	void createKeys();
	shared_ptr<RSA> retrieveKey(int key_id);
	vector<int> findOnionPath(int target_id);
	void sendMPNP(int target_id, string payload);
};

struct RelaySession {
	int to;
	int from;
	int session;
	time_point creation;
	string key;

	RelaySession(int to, int from, int session);
	RelaySession(int to, int from, int session, string k);
};

class RootNode {
public:
	shared_ptr<Node> node;
	shared_ptr<OpenConnection> admin;
	thread handle_thread;
	thread detached_thread;
	DHT dht;
	vector<PunchholePair> punchhole_pairs;
	vector<uint8_t> port_use = { false, false, false };
	mutex punchhole_pairs_mutex;

	RootNode();
	RootNode(shared_ptr<Node> n);
	void createDHT();
	void overtakeDHT();
	void handleThread();
	void handleMessage(Message message);
	void handleMessage(string message);
	void handleMessage(shared_ptr<Connection> connection, string message);
	void handleMessage(RelaySession relay_session, shared_ptr<Connection> connection, string message);
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