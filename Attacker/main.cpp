#include <iostream>
#include <thread>
#include <chrono>

#include "node.h"
#include "storage.h"

using namespace std;

int main() {
	cout << "Loading..." << endl << endl;

	Storage::initialize();
	shared_ptr<Node> node = make_shared<Node>();

	string input;
	while (!node->console->exit) {
		getline(cin, input);
		node->console->handleInput(input);
	}

	return 0;
}