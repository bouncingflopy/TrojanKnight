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
struct ChessInvite;

class Node {
public:
	int id = -1;
	string ip;
	vector<shared_ptr<Connection>> connections;
	shared_ptr<Connection> rootConnection;
	shared_ptr<Connection> punchholeRC;
	shared_ptr<Connection> chess_connection;
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
	shared_ptr<thread> reroot_thread;
	DHT dht;
	vector<RelaySession> relay_sessions;
	mutex punchholeRC_mutex;
	mutex connections_mutex;
	shared_ptr<time_point> joined_time;
	shared_ptr<ChessInvite> outgoing_invite;
	vector<shared_ptr<ChessInvite>> incoming_invites;
	vector<shared_ptr<ChessInvite>> outgoing_invites_history;

	Node();
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
	void changeName(string name);
	void sendInvite(int target_id);
	void cancelInvite();
	void acceptInvite();
	void rejectInvite();
	bool checkConnectedToNode(int target_id);
	shared_ptr<Connection> getConnectionToNode(int target_id);
	void createGame(int game);
};

struct RelaySession {
	int to;
	int from;
	int session;
	time_point creation;

	RelaySession(int to, int from, int session);
};

struct ChessInvite {
	int to;
	int from;
	int game;

	ChessInvite(int to, int from, int game);
};

// wan holepunching
// wan reroot
// wan lobby
// wan chess
// wan timers

// upnp

// add private ip connection
// what if node sends root dht update (connect and disconnect) but root is rerooting or doesnt get it?
// root check no one takes ddns, if im empty connect to them (mid reroot / root without pf)
// add ttl for rootconnection?
// node joining while reroot
// check for name change race condition with api call to root
// lobby cooldown
// sending lobby request and then they leave or dht order changes (keep selected)
// dont display players who are in game in the lobby
// board error when sending packet to disconnected opponent
// lobby changing dht when focused and red buttoned on a player that moved around in lobby or disconnected (also not red)
// full lobby trying to open chess connection crash

/*

rpnp
v	dht request
v	dht join
v	dht connect 0 1
v	dht disconnect 0 1
v	dht leave 1
v	dht rename 1 username
v	punchhole request 0 1
v	punchhole fail 0 1

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
v	chess invite [from] 1 [game] 1
v	chess cancel [game] 1
v	chess accept [game] 1
v	chess reject [game] 1
v	chess start [game] 1

*/

/*

keepalive
syn
ack

*/

// make all pointers shared_ptrs in chess
// make chess board size based on computer size
// endcryption public key private key
// node rootnode connection openconnection destructors for closing threads and queues?
// delete unnecessary files (+ github)
// add cool features like private key public key, signature, kademillia
// optimize code and run times
// add comments and documentation
// patch vulnerabilies and improve security
// resize window

class RootNode {
public:
	shared_ptr<Node> node;
	shared_ptr<OpenConnection> admin;
	thread handle_thread;
	thread detached_thread;
	DHT dht;
	vector<PunchholePair> punchhole_pairs;
	vector<uint8_t> port_use = {false, false, false};
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