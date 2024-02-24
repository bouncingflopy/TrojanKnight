#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H

#include <string>
#include <chrono>

using namespace std;
typedef std::basic_string<char> string;
typedef std::chrono::high_resolution_clock::time_point time_point;

const int ROOT_PORT = 13370; // open ports 13370-13373

// change account to anonymous one
const string DDNS_URL = "p2pchat.freeddns.org";
const string DDNS_ID = "10013754";
const string DDNS_API_KEY = "cb46VUZ4Ub3T3537266Z54Y67b5cf453";

const int SESSION_RANGE = 1000;
const int SESSION_ID_MOD = 1000;

const int HANDSHAKE_TIME = 4;
const int HANDSHAKE_FREQUENCY = 500;
const int KEEPALIVE_FREQUENCY = 2;
const int KEEPALIVE_DETECTION = 8;
const int SESSION_TTL = 15;
const int LOOKOUT_CHECK_FREQUENCY = 500;
const int DETACHED_DETECTION = 30;
const int DETACHED_CHECK_FREQUENCY = 5;
const int REROOT_CHECK_FREQUENCY = 3;

#endif