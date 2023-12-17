#ifndef CONNECTION_H
#define CONNECTION_H

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE

#include <asio.hpp>
#include <string>
#include <vector>
#include <queue>
#include <chrono>
#include <iostream>

#include "networksettings.h"
#include "node.h"

using namespace std;
typedef std::basic_string<char> string;

class Node;

class Connection {
public:
	int my_port;
	string remote_ip;
	int remote_port;
	bool connected = false;
	bool force_close = false;
	asio::io_context context;
	thread context_thread;
	asio::ip::tcp::socket* socket;
	asio::ip::tcp::socket* client_socket;
	asio::ip::tcp::socket* server_socket;
	asio::ip::tcp::endpoint local_endpoint;
	vector<char> read_buffer = vector<char>(1024);
	queue<string> incoming_messages;

	Connection(int mp, string ri, int rp, int time);
	void close();
	void asyncReadData();
	void writeData(string data);
	void serverThread();
	void clientThread();
};

#endif