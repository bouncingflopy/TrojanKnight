#include <chrono>

#include "connection.h"

using namespace std;

Connection::Connection(string i, int p, int d) : ip(i), port(p), id(d) {
	asio::io_context::work idleWork(context);
	context_thread = thread([&]() {context.run();});

	socket = new asio::ip::udp::socket(context);
	socket->open(asio::ip::udp::v4());

	asio::ip::udp::endpoint e = asio::ip::udp::endpoint(asio::ip::make_address(ip), port);
	connect(e);
}

Connection::~Connection() {
	context.stop();
	context_thread.join();

	if (socket->is_open()) socket->close();
	delete socket;
}

void Connection::asyncReadData() {
	socket->async_receive_from(asio::buffer(read_buffer.data(), read_buffer.size()), endpoint,
		[&](error_code ec, size_t length) {
			if (!ec) {
				incoming_messages.push(string(read_buffer.begin(), read_buffer.begin() + length));
				asyncReadData();
			}
		}
	);
}

void Connection::writeData(string data) {
	socket->send_to(asio::buffer(data.data(), data.size()), endpoint);
}

void Connection::connect(asio::ip::udp::endpoint e) {
	connected = false;
	socket->cancel();

	endpoint = e;

	asio::error_code ec;
	socket->connect(endpoint, ec); // might not need this, check if holepunching works, but need connect bool

	if (ec) {
		cout << ec << endl;
	}
	else {
		connected = true;
		keepalive = chrono::high_resolution_clock::now();
		asyncReadData();
	}
}

void Connection::change(string i, int p, int d) {
	ip = i;
	port = p;
	id = d;

	asio::ip::udp::endpoint e = asio::ip::udp::endpoint(asio::ip::make_address(ip), port);
	connect(e);
}

OpenConnection::OpenConnection() {
	asio::io_context::work idleWork(context);
	context_thread = thread([&]() {context.run();});

	local_endpoint = asio::ip::udp::endpoint(asio::ip::udp::v4(), ROOT_PORT);
	socket = new asio::ip::udp::socket(context, local_endpoint);

	asyncReceive();
}

void OpenConnection::writeData(asio::ip::udp::endpoint endpoint, string data) {
	socket->send_to(asio::buffer(data.data(), data.size()), endpoint);
}

void OpenConnection::asyncReceive() {
	asio::ip::udp::endpoint* endpoint = new asio::ip::udp::endpoint();
	socket->async_receive_from(asio::buffer(read_buffer.data(), read_buffer.size()), *endpoint,
		[&](error_code ec, size_t length) {
			if (!ec) {
				Message message;
				message.endpoint = *endpoint;
				message.message = string(read_buffer.begin(), read_buffer.begin() + length);

				incoming_messages.push(message);
			}

			asyncReceive();
		});
}