#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H

#include <string>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>

using namespace std;
typedef std::chrono::high_resolution_clock::time_point time_point;

const int ROOT_PORT = 13370; // open ports 13370-13373

// change account to anonymous one
const string DDNS_URL = "p2pchat.freeddns.org";
const string DDNS_ID = "10013754";
const string DDNS_API_KEY = "cb46VUZ4Ub3T3537266Z54Y67b5cf453";

const int ROOT_CONNECTION_TTL = 10;
const int HANDSHAKE_TIME = 8;
const int HANDSHAKE_FREQUENCY = 250;
const int KEEPALIVE_FREQUENCY = 2;
const int KEEPALIVE_DETECTION = 8;
const int SESSION_TTL = 15;
const int LOOKOUT_CHECK_FREQUENCY = 500;
const int DETACHED_DETECTION = 30;
const int DETACHED_CHECK_FREQUENCY = 5;
const int REROOT_CHECK_FREQUENCY = 3;
const int HANDLE_FREQUENCY = 50;
const int PUNCHHOLERC_TTL = 8;
const int PUNCHHOLERC_TTL_JITTER = 3;

#endif