#define CURL_STATICLIB

#include <iostream>
#include <sstream>

#include "connection.h"
#include "node.h"

ICMPConnection::ICMPConnection(Node* n) : node(n) {
	asio::io_context::work idleWork(context);
	context_thread = thread([&]() {context.run();});
	icmp_socket = new asio::ip::icmp::socket(context);
	icmp_socket->open(asio::ip::icmp::v4());
	icmp_socket->set_option(asio::ip::icmp::socket::reuse_address(true));

	empty_ip = { 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04 };
	empty_echo = { 0x08, 0x00, 0xf7, 0xff, 0x00, 0x00, 0x00, 0x00 };
	empty_ttl = { 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	//send_thread = thread([&]() {asyncSendICMP();});

	asyncReceiveICMP();
}

void ICMPConnection::asyncSendICMP() { //\x58\xfc\x20\x1d\x8a\xdf\x3c\x7c\x3f\xba\x2b\x5a\x08\x00
	const char* packet = "E\0\0\x1c\0\0\0\0@\x01\xe0\xa2M\x89H\xb0\x01\x02\x03\x04\b\0\xf7\xff\0\0\0\0";

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed." << std::endl;
	}

	SOCKET rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (rawSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create raw socket." << std::endl;
		WSACleanup();
	}

	// Set the IP_HDRINCL socket option to include the custom IP header
	int enable = 1;
	if (setsockopt(rawSocket, IPPROTO_IP, IP_HDRINCL, reinterpret_cast<char*>(&enable), sizeof(enable)) == SOCKET_ERROR) {
		std::cerr << "Failed to set IP_HDRINCL option." << std::endl;
		closesocket(rawSocket);
		WSACleanup();
	}

	sockaddr_in destAddress;
	destAddress.sin_family = AF_INET;
	const char* destIP = "8.8.8.8"; // Replace with the actual destination IP
	if (inet_pton(AF_INET, destIP, &(destAddress.sin_addr)) <= 0) {
		std::cerr << "Invalid destination IP address." << std::endl;
		closesocket(rawSocket);
		WSACleanup();
	}

	int result = sendto(rawSocket, packet, strlen(packet), 0, reinterpret_cast<sockaddr*>(&destAddress), sizeof(destAddress));
	if (result == SOCKET_ERROR) {
		std::cerr << "sendto failed." << std::endl;
		int error = WSAGetLastError();
		std::cerr << "Failed to create raw socket. Error code: " << error << std::endl;
	}

	int error = WSAGetLastError();
	std::cerr << "Winsock error code: " << error << std::endl;
	cout << "sent" << endl;

	closesocket(rawSocket);
	WSACleanup();

	//while (1) {
	//	raw_socket->send_to(asio::buffer(echo_packet), dead_endpoint);
	//	this_thread::sleep_for(chrono::milliseconds(ICMP_HOLEPUNCHING_FREQUENCY));
	//}
}

void ICMPConnection::asyncReceiveICMP() {
	icmp_socket->async_receive(asio::buffer(receive_buffer),
		[&](error_code ec, size_t length) {
			string received(receive_buffer.begin(), receive_buffer.begin() + length);
			size_t index = received.find_last_of("pnp");

			if (index != string::npos) {
				index -= 2;
				string pnp(receive_buffer.begin() + index, receive_buffer.begin() + length);
				vector<string> words;
				istringstream stream(pnp);
				string temp;
				while (getline(stream, temp, ' ')) words.push_back(temp);

				if (words[1] == "request" && words.size() == 4) {
					acceptConnect(words[2], stoi(words[3]));
				}
				else if (words[1] == "accept" && words.size() == 7) {
					actConnect(words[2], stoi(words[3]), stoi(words[6]));
				}
			}

			asyncReceiveICMP();
		}
	);
}

void ICMPConnection::calculateChecksum(vector<uint8_t>& packet, int index) {
	uint32_t sum = 0;

	for (int i = 0; i < packet.size(); i += 2) {
		sum += static_cast<uint16_t>(packet[i] << 8);

		if (i + 1 < packet.size()) {
			sum += static_cast<uint16_t>(packet[i + 1]);
		}
	}

	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	uint16_t checksum = static_cast<uint16_t>(~sum);
	uint8_t upper_sum = static_cast<uint8_t>(checksum >> 8);
	uint8_t lower_sum = static_cast<uint8_t>(checksum & 0xFF);

	packet[index] = upper_sum;
	packet[index + 1] = lower_sum;
}

vector<uint8_t> ICMPConnection::ipEchoPacket(string src_ip) {
	vector<uint8_t> ip = empty_ip;

	vector<uint8_t> bytes;
	istringstream stream(src_ip);
	string temp;
	while (getline(stream, temp, '.')) bytes.push_back(static_cast<uint8_t>(stoi(temp)));
	for (int i = 0; i < 4; i++) {
		ip[12 + i] = bytes[i];
	}

	ip.reserve(empty_echo.size());
	ip.insert(ip.end(), empty_echo.begin(), empty_echo.end());

	uint16_t ip_length = static_cast<uint16_t>(ip.size());
	ip[2] = static_cast<uint8_t>(ip_length >> 8);
	ip[3] = static_cast<uint8_t>(ip_length & 0xFF);

	calculateChecksum(ip, 10);

	return ip;
}

vector<uint8_t> ICMPConnection::fakePacket(string payload_string, string remote_ip) {
	vector<uint8_t> packet = empty_ttl;
	vector<uint8_t> ip = empty_ip;
	
	vector<uint8_t> bytes;
	istringstream stream(remote_ip);
	string temp;
	while (getline(stream, temp, '.')) bytes.push_back(static_cast<uint8_t>(stoi(temp)));
	for (int i = 0; i < 4; i++) {
		ip[12 + i] = bytes[i];
	}

	ip.reserve(empty_echo.size());
	ip.insert(ip.end(), empty_echo.begin(), empty_echo.end());

	//vector<char> payload(payload_string.begin(), payload_string.end());
	//ip.reserve(payload.size());
	//ip.insert(ip.end(), payload.begin(), payload.end());

	uint16_t ip_length = static_cast<uint16_t>(ip.size());
	ip[2] = static_cast<uint8_t>(ip_length >> 8);
	ip[3] = static_cast<uint8_t>(ip_length & 0xFF);

	calculateChecksum(ip, 10);

	packet.reserve(ip.size());
	packet.insert(packet.end(), ip.begin(), ip.end());

	calculateChecksum(packet, 2);

	return packet;
}

void ICMPConnection::requestConnect(string remote_ip) {
	asio::ip::icmp::endpoint endpoint(asio::ip::make_address(remote_ip), 0);
	
	int my_port = 13134; // implement random port
	string payload_string = "pnp request " + node->ipv4 + " " + to_string(my_port);

	icmp_socket->send_to(asio::buffer(fakePacket(payload_string, remote_ip)), endpoint);
}

void ICMPConnection::acceptConnect(string remote_ip, int remote_port) { // make sure accepted is requested for multiple nodes in same LAN situation
	asio::ip::icmp::endpoint endpoint(asio::ip::make_address(remote_ip), 0);

	int my_port = 13132; // implement random port
	string payload_string = "pnp accept " + node->ipv4 + " " + to_string(my_port) + "request " + remote_ip + " " + to_string(remote_port);

	icmp_socket->send_to(asio::buffer(fakePacket(payload_string, remote_ip)), endpoint);

	node->connections.push_back(new Connection(my_port, remote_ip, remote_port));
}

void ICMPConnection::actConnect(string remote_ip, int remote_port, int my_port) {
	node->connections.push_back(new Connection(my_port, remote_ip, remote_port));
}