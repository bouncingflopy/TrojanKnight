#include <chrono>

#include "connection.h"

using namespace std;

Connection::Connection() {}

Connection::Connection(string i, int p, int d) : ip(i), port(p), id(d) {
	asio::io_context::work idleWork(context);
	context_thread = thread([&]() {context.run();});

	socket = make_shared<asio::ip::udp::socket>(context);
	socket->open(asio::ip::udp::v4());
	/*asio::ip::udp::endpoint local_endpoint = asio::ip::udp::endpoint(asio::ip::make_address("0.0.0.0"), 0);
	socket->bind(local_endpoint);*/

	asio::ip::udp::endpoint e = asio::ip::udp::endpoint(asio::ip::make_address(ip), port);
	connect(e);
}

Connection::~Connection() {
	context.stop();
	context_thread.join();

	if (socket->is_open()) socket->close();
	socket.reset();
}

bool Connection::checkNodeProtocol(string data) {
	return (data.find("pnp") == 0 || data.find("rpnp") == 0 || data.find("cpnp") == 0);
}

void Connection::handleConnectionMessage(string data) {
	if (data == "keepalive") {
		keepalive = chrono::high_resolution_clock::now();
	}
	else if (data == "syn") {
		connected = true;
		writeData("ack");
	}
	else if (data == "ack") {
		connected = true;
	}
}

void Connection::asyncReadData() {
	fill(read_buffer.begin(), read_buffer.end(), 0);

	socket->async_receive_from(asio::buffer(read_buffer.data(), read_buffer.size()), endpoint,
		[&](error_code ec, size_t length) {
			if (!ec) {
				vector<char>::iterator end_of_data = find(read_buffer.begin(), read_buffer.end(), '\0');
				string data(read_buffer.begin(), end_of_data);

				if (checkNodeProtocol(data)) incoming_messages.push(data);
				else handleConnectionMessage(data);
			}

			asyncReadData();
		}
	);
}

void Connection::writeData(string data) {
	socket->send_to(asio::buffer(data.data(), data.size()), endpoint);
}

void Connection::handshake() {
	time_point start = chrono::high_resolution_clock::now();
	time_point now;
	int time_passed = 0;

	while (!connected && time_passed < HANDSHAKE_TIME) {
		writeData("syn");

		this_thread::sleep_for(chrono::milliseconds(HANDSHAKE_FREQUENCY));

		now = chrono::high_resolution_clock::now();
		time_passed = chrono::duration_cast<chrono::seconds>(now - start).count();
	}
}

void Connection::connect(asio::ip::udp::endpoint e) {
	connected = false;
	socket->cancel();

	endpoint = e;

	error_code ec;
	socket->connect(endpoint, ec);
	
	asyncReadData();
	handshake();

	if (connected) {
		keepalive = chrono::high_resolution_clock::now();
	}
}

void Connection::change(string i, int p, int d) {
	ip = i;
	port = p;
	id = d;

	asio::ip::udp::endpoint e = asio::ip::udp::endpoint(asio::ip::make_address(ip), port);
	connect(e);
}

void Connection::releaseChess() {
	chess_connection = false;
	board.reset();
}

RootConnection::RootConnection(string i, int p, int d, int my_port) {
	ip = i;
	port = p;
	id = d;

	asio::io_context::work idleWork(context);
	context_thread = thread([&]() {context.run();});

	socket = make_shared<asio::ip::udp::socket>(context);
	socket->open(asio::ip::udp::v4());
	asio::ip::udp::endpoint local_endpoint = asio::ip::udp::endpoint(asio::ip::make_address("127.0.0.1"), my_port); // lan
	socket->bind(local_endpoint);

	connect();
}

void RootConnection::connect() {
	asyncReadData();
	
	time_point start = chrono::high_resolution_clock::now();
	time_point now;
	int time_passed = 0;

	while (!connected && time_passed < HANDSHAKE_TIME) {
		this_thread::sleep_for(chrono::milliseconds(HANDSHAKE_FREQUENCY));

		now = chrono::high_resolution_clock::now();
		time_passed = chrono::duration_cast<chrono::seconds>(now - start).count();
	}

	if (connected) {
		keepalive = chrono::high_resolution_clock::now();
	}
}

OpenConnection::OpenConnection() {
	asio::io_context::work idleWork(context);
	context_thread = thread([&]() {context.run();});

	local_endpoint = asio::ip::udp::endpoint(asio::ip::udp::v4(), ROOT_PORT);
	socket = make_shared<asio::ip::udp::socket>(context, local_endpoint);

	asyncReceive();
}

void OpenConnection::writeData(asio::ip::udp::endpoint endpoint, string data) {
	socket->send_to(asio::buffer(data.data(), data.size()), endpoint);
}

bool OpenConnection::checkNodeProtocol(string data) {
	return data.find("rpnp") == 0;
}

void OpenConnection::handleConnectionMessage(Message data) {
	if (data.message == "syn") {
		writeData(data.endpoint, "ack");
	}
}

void OpenConnection::asyncReceive() {
	fill(read_buffer.begin(), read_buffer.end(), 0);

	socket->async_receive_from(asio::buffer(read_buffer.data(), read_buffer.size()), receiving_endpoint,
		[&](error_code ec, size_t length) {
			if (!ec) {
				Message message;
				message.endpoint = receiving_endpoint;

				vector<char>::iterator end_of_data = find(read_buffer.begin(), read_buffer.end(), '\0');
				message.message = string(read_buffer.begin(), end_of_data);
				
				if (checkNodeProtocol(message.message)) incoming_messages.push(message);
				else handleConnectionMessage(message);
			}

			asyncReceive();
		});
}