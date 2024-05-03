#include <chrono>
#include <string>

#include "connection.h"

using namespace std;

Connection::Connection() {}

Connection::Connection(string i, int p, int d) : ip(i), port(p), id(d) {
	asio::io_context::work idleWork(context);
	context_thread = thread([&]() {context.run();});

	socket = make_shared<asio::ip::udp::socket>(context);
	socket->open(asio::ip::udp::v4());
	asio::ip::udp::endpoint local_endpoint = asio::ip::udp::endpoint(asio::ip::make_address("127.0.0.1"), 0); // lan
	socket->bind(local_endpoint);

	asio::ip::udp::endpoint e = asio::ip::udp::endpoint(asio::ip::make_address(ip), port);
	connect(e);
}

Connection::Connection(string i, int p, int d, shared_ptr<RSA> dk) : Connection(i, p, d) {
	decryption_key = dk;
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

bool Connection::checkConnectionProtocol(string data) {
	return (data.find("keepalive") == 0 || data.find("syn") == 0 || data.find("ack") == 0);
}

void Connection::handleConnectionMessage(string data) {
	if (data == "keepalive") {
		keepalive = chrono::high_resolution_clock::now();
	}
	else if (data == "syn") {
		connected = true;
		writeProtocolless("ack");
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

				if (data.substr(0, 6) == "epnpa\n") {
					end_of_data = find_end(read_buffer.begin(), read_buffer.end(), "\nepnpa", "\nepnpa" + 6);
					data = string(read_buffer.begin(), end_of_data);
					data = data.substr(6, data.length() - 6);

					data = Encryption::decryptAES(data, AES_MASTER_KEY);
				}

				if (data.substr(0, 6) == "epnpr\n") {
					fill(read_buffer.begin(), read_buffer.end(), 0);
					copy(data.begin(), data.end(), read_buffer.begin());

					end_of_data = find_end(read_buffer.begin(), read_buffer.end(), "\nepnpr", "\nepnpr" + 6);
					data = string(read_buffer.begin(), end_of_data);
					data = data.substr(6, data.length() - 6);

					data = Encryption::decrypt(data, decryption_key);
				}
				
				if (checkConnectionProtocol(data)) handleConnectionMessage(data);
				else if (checkNodeProtocol(data)) incoming_messages.push(data);
			}

			asyncReadData();
		}
	);
}

void Connection::writeData(string data) {
	string payload = data;

	if (encryption_key && !checkConnectionProtocol(data)) {
		payload = "epnpr\n" + Encryption::encrypt(data, encryption_key) + "\nepnpr";
	}

	writePlain(payload);
}

void Connection::writePlain(string data) {
	data = "epnpa\n" + Encryption::encryptAES(data, AES_MASTER_KEY) + "\nepnpa";

	writeProtocolless(data);
}

void Connection::writeProtocolless(string data) {
	socket->send_to(asio::buffer(data.data(), data.size()), endpoint);
}

void Connection::handshake() {
	time_point start = chrono::high_resolution_clock::now();
	time_point now;
	int time_passed = 0;

	while (!connected && time_passed < HANDSHAKE_TIME) {
		writeProtocolless("syn");

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

void Connection::change(string i, int p, int d, shared_ptr<RSA> dk) {
	ip = i;
	port = p;
	id = d;
	decryption_key = dk;

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

RootConnection::RootConnection(string i, int p, int d, int my_port, shared_ptr<RSA> dk) : RootConnection(i, p, d, my_port) {
	decryption_key = dk;
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
	data = "epnpa\n" + Encryption::encryptAES(data, AES_MASTER_KEY) + "\nepnpa";

	writeProtocolless(endpoint, data);
}

void OpenConnection::writeProtocolless(asio::ip::udp::endpoint endpoint, string data) {
	socket->send_to(asio::buffer(data.data(), data.size()), endpoint);
}

bool OpenConnection::checkNodeProtocol(string data) {
	return data.find("rpnp") == 0;
}

void OpenConnection::handleConnectionMessage(Message data) {
	if (data.message == "syn") {
		writeProtocolless(data.endpoint, "ack");
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
				string data(read_buffer.begin(), end_of_data);
				
				if (data.substr(0, 6) == "epnpa\n") {
					end_of_data = find_end(read_buffer.begin(), read_buffer.end(), "\nepnpa", "\nepnpa" + 6);
					data = string(read_buffer.begin(), end_of_data);
					data = data.substr(6, data.length() - 6);

					data = Encryption::decryptAES(data, AES_MASTER_KEY);
				}

				message.message = data;

				if (checkNodeProtocol(message.message)) incoming_messages.push(message);
				else handleConnectionMessage(message);
			}

			asyncReceive();
		});
}