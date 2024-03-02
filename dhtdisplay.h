#ifndef DHTDISPLAY_H
#define DHTDISPLAY_H

#include "node.h"

#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>

using namespace std;

void dhtDisplay(vector<shared_ptr<Node>>& nodes);
void dhtDisplay(const DHT& dht);

#endif