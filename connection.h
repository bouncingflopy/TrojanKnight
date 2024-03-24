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
#include "board.h"

using namespace std;

class Node;
class Board;

class Connection {
public:
	int id;
	string ip;
	int port;
	bool connected = false;
	shared_ptr<asio::ip::udp::socket> socket;
	asio::ip::udp::endpoint endpoint;
	asio::io_context context;
	thread context_thread;
	vector<char> read_buffer = vector<char>(1024);
	queue<string> incoming_messages;
	time_point keepalive;
	bool chess_connection = false;
	shared_ptr<Board> board;

	Connection();
	Connection(string i, int p, int id);
	~Connection();
	bool checkNodeProtocol(string data);
	void handleConnectionMessage(string data);
	void asyncReadData();
	void writeData(string data);
	void handshake();
	void connect(asio::ip::udp::endpoint e);
	void change(string i, int p, int id);
	void releaseChess();
};

struct Message {
	asio::ip::udp::endpoint endpoint;
	string message;
};

class RootConnection : public Connection {
public:
	RootConnection(string i, int p, int id, int my_port);
	void connect();
};

class OpenConnection {
public:
	asio::ip::udp::endpoint local_endpoint;
	asio::ip::udp::endpoint receiving_endpoint;
	shared_ptr<asio::ip::udp::socket> socket;
	asio::io_context context;
	thread context_thread;
	vector<char> read_buffer = vector<char>(1024);
	queue<Message> incoming_messages;

	OpenConnection();
	void writeData(asio::ip::udp::endpoint endpoint, string data);
	bool checkNodeProtocol(string data);
	void handleConnectionMessage(Message data);
	void asyncReceive();
};

#endif