#include <chrono>

#include "connection.h"

using namespace std;

Connection::Connection(int mp, string ri, int rp, int time) : my_port(mp), remote_ip(ri), remote_port(rp) {
	asio::io_context::work idleWork(context);
	local_endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), my_port);
	context_thread = thread([&]() {context.run();});
	
	thread server_thread([this]() {serverThread();});
	thread client_thread([this]() {clientThread();});

	for (int i = 0; i < time; i++) {
		if (connected) break;
		this_thread::sleep_for(1s);
	} // make all timers async

	force_close = true;
	server_thread.join(); // what if both at same time
	client_thread.join();

	if (connected) {
		if (socket == server_socket) client_socket->close();
		else server_socket->close();

		asyncReadData();
		cout << "connected to " << ri << endl; // debug
	}
}

void Connection::close() {
	context.stop();
	context_thread.join();

	client_socket->close();
	server_socket->close();
}

void Connection::asyncReadData() {
	socket->async_read_some(asio::buffer(read_buffer.data(), read_buffer.size()),
		[&](error_code ec, size_t length) {
			if (!ec) {
				incoming_messages.push(string(read_buffer.begin(), read_buffer.begin() + length));
				// if doesnt start with pnp, concate to last message
				asyncReadData();
			}
		}
	);
}

void Connection::writeData(string data) {
	socket->write_some(asio::buffer(data.data(), data.size()));
}

void Connection::serverThread() {
	server_socket = new asio::ip::tcp::socket(context);

	asio::ip::tcp::acceptor acceptor = asio::ip::tcp::acceptor(context, local_endpoint);

	acceptor.async_accept(*server_socket,
		[&](asio::error_code ec) {
			if (!ec) {
				socket = server_socket;
				connected = true;
			}
		}
	);

	while (!connected && !force_close) {
		this_thread::sleep_for(chrono::milliseconds(TCP_HOLEPUNCHING_FREQUENCY));
	}
}

void Connection::clientThread() {
	client_socket = new asio::ip::tcp::socket(context);
	client_socket->open(asio::ip::tcp::v4());
	client_socket->set_option(asio::socket_base::reuse_address(true));
	client_socket->bind(local_endpoint);

	asio::ip::tcp::endpoint endpoint(asio::ip::make_address(remote_ip), remote_port);

	while (!connected && !force_close) {
		client_socket->async_connect(endpoint,
			[&](asio::error_code ec) {
				if (!ec) {
					socket = client_socket;
					connected = true;
				}
			}
		);

		this_thread::sleep_for(chrono::milliseconds(TCP_HOLEPUNCHING_FREQUENCY));
	}
}