#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H

#include <string>

using namespace std;
typedef std::basic_string<char> string;

const int DEFAULT_PORT = 42000;
const int LAST_DDNS_CONNECTION_WAIT_TIME = 5;
const int LAST_DDNS_EXTRA_CONNECTION_WAIT_TIME = 20;
const int NEXT_DDNS_CONNECTION_WAIT_TIME = 10;
const int NEXT_DDNS_CHECK_FREQUENCY = 8;
const int TCP_HOLEPUNCHING_FREQUENCY = 500;

// change account to anonymous one
const string DDNS_URL = "p2pchat.freeddns.org";
const string DDNS_ID = "10013754";
const string DDNS_API_KEY = "cb46VUZ4Ub3T3537266Z54Y67b5cf453";

#endif