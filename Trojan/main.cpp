#include <iostream>
#include <thread>
#include <chrono>

#include "node.h"
#include "shell.h"
#include "storage.h"

using namespace std;

// todo
// mpnp noise

int main() {
	Storage::initialize();

	shared_ptr<Node> node = make_shared<Node>();

	while (true) {
		this_thread::sleep_for(chrono::seconds(1));
	}

	return 0;
}