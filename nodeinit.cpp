#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#define CURL_STATICLIB

#include <iostream>
#include <asio.hpp>
#include <chrono>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "node.h"
#include "connection.h"

using namespace std;

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
};

static size_t writeData(void* buffer, size_t size, size_t nmemb, void* userp) {return size * nmemb;}

Node::Node() {
	ipv4 = getIPv4();
	last_ddns = getDDNS(); // multiple roots
	cout << "Root: " << last_ddns << endl;
	
	if (last_ddns != ipv4) {
		cout << "Trying to connect..." << endl; // debug
		Connection* last_ddns_connection = new Connection(DEFAULT_PORT, last_ddns, DEFAULT_PORT, LAST_DDNS_CONNECTION_WAIT_TIME);

		if (last_ddns_connection->connected) {
			connections.push_back(last_ddns_connection);
		}
		else {
			last_ddns_connection->close();

			becomeRoot();

			UPnP_enabled = checkUPnP();
			if (UPnP_enabled) {
				createUPnP(); // close
			}
			else {
				next_ddns_thread = thread([this]() {connectNextDDNS();});
				last_ddns_thread = thread([this]() {connectLastDDNS();});
			}
		}
	}
	else {
		UPnP_enabled = checkUPnP();
		if (UPnP_enabled) {
			createUPnP(); // close
		}
		else {
			next_ddns_thread = thread([this]() {connectNextDDNS();});
		}
	}

	cout << "Connected" << endl;
}

string Node::getIPv4() {
	CURL* curl;
	CURLcode res;
	string response;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org?format=json");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl; // debug

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	cout << "IPv4: " << nlohmann::json::parse(response)["ip"] << endl;

	return nlohmann::json::parse(response)["ip"];
}

string Node::getDDNS() {
	CURL* curl;
	CURLcode res;
	string response;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.dynu.com/v2/dns");

		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Host: api.dynu.com");
		headers = curl_slist_append(headers, "accept: application/json");
		headers = curl_slist_append(headers, ("API-Key: " + DDNS_API_KEY).c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl; // debug

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return nlohmann::json::parse(response)["domains"][0]["ipv4Address"];
}

void Node::becomeRoot() {
	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, ("https://api.dynu.com/v2/dns/" + DDNS_ID).c_str());

		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Host: api.dynu.com");
		headers = curl_slist_append(headers, "accept: application/json");
		headers = curl_slist_append(headers, ("API-Key: " + DDNS_API_KEY).c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		string json_data = "{\"name\":\"" + DDNS_URL + "\",\"ipv4Address\":\"" + ipv4 + "\"}";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl; // debug

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	is_root = true;
}

bool Node::checkUPnP() {
	return false; // implement
}

void Node::createUPnP() {
	
}

void Node::connectNextDDNS() {
	string ddns_response = getDDNS();

	while (ddns_response == ipv4) {
		this_thread::sleep_for(chrono::seconds(NEXT_DDNS_CONNECTION_WAIT_TIME));
		ddns_response = getDDNS();
	}

	Connection* connection = new Connection(DEFAULT_PORT, ddns_response, DEFAULT_PORT, NEXT_DDNS_CONNECTION_WAIT_TIME);

	if (connection->connected) connections.push_back(connection);
	else connection->close();
}

void  Node::connectLastDDNS() {
	Connection* connection = new Connection(DEFAULT_PORT, last_ddns, DEFAULT_PORT, LAST_DDNS_EXTRA_CONNECTION_WAIT_TIME);

	if (connection->connected) connections.push_back(connection);
	else connection->close();
}