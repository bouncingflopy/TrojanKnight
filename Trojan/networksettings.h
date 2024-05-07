#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H

#include <string>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>

using namespace std;
typedef std::chrono::high_resolution_clock::time_point time_point;

const int MESSAGE_BUFFER_SIZE = 4096;

const int ROOT_PORT = 18870; // open ports 18870-18873

const string AES_MASTER_KEY = "\xcb\xba\x93\x96\x3c\x94\x33\x10\x0f\xb6\x7c\x13\xd3\x6d\x47\x09\xe6\x74\x69\x9c\xd6\x20\x4a\x49";

// change account to anonymous one
const string DDNS_URL = "p2pchat.freeddns.org"; // lan
const string DDNS_ID = "10013754";
const string DDNS_API_KEY = "cb46VUZ4Ub3T3537266Z54Y67b5cf453";

const int ROOT_CONNECTION_TTL = 10;
const int HANDSHAKE_TIME = 8;
const int HANDSHAKE_FREQUENCY = 250;
const int KEEPALIVE_FREQUENCY = 2;
const int KEEPALIVE_DETECTION = 8;
const int SESSION_TTL = 60;
const int LOOKOUT_CHECK_FREQUENCY = 500;
const int DETACHED_DETECTION = 30;
const int DETACHED_CHECK_FREQUENCY = 5;
const int REROOT_CHECK_FREQUENCY = 3;
const int HANDLE_FREQUENCY = 50;
const int PUNCHHOLERC_TTL = 8;
const int PUNCHHOLERC_TTL_JITTER = 3;
const int KEY_RETRIEVE_CHECK_FREQUENCY = 1;
const int NAME_CHECK_FREQUENCY = 3;

const int SHELL_CHECK_INCOMING_FREQUENCY = 100;
const int SHELL_READ_FREQUENCY = 100;

#endif