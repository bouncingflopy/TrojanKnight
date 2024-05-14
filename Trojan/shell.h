#ifndef SHELL_H
#define SHELL_H

#include <memory>
#include <queue>
#include <atomic>
#include <thread>
#include <fstream>

#include "node.h"

using namespace std;

class Node;
struct RelaySession;

class Shell {
public:
	shared_ptr<Node> node;
	shared_ptr<RelaySession> session;
	FILE* pipe;
	thread execute_thread;
	queue<string> commands;
	string output_path;
	bool signal = false;

	Shell(shared_ptr<Node> n);
	void invoke();
	void terminate();
	void command();
	void mpnp(string data, shared_ptr<RelaySession> s);
};

/*

mpnp
v	invoke
v	terminate
v	command
v		...
v	noise
v		...

*/

#endif SHELL_H