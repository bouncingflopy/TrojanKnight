#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>

#include "connection.h"
#include "networksettings.h"

class Connection;
class ICMPConnection;

class Node {
public:
	vector<Connection*> connections;
	string ipv4;
	string last_ddns;
	bool is_root;
	bool UPnP_enabled;
	thread last_ddns_thread;
	thread next_ddns_thread;

	Node();
	string getIPv4();
	string getDDNS();
	void becomeRoot();
	bool checkUPnP();
	void createUPnP();
	void connectNextDDNS();
	void connectLastDDNS();
};

#endif