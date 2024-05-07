#include <iostream>
#include <thread>
#include <chrono>

#include "node.h"
#include "shell.h"
#include "storage.h"

using namespace std;

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

void disableConsoleInput() {
	HWND hwndConsole = GetConsoleWindow();
	EnableWindow(hwndConsole, FALSE);
}

void enableConsoleInput() {
	HWND hwndConsole = GetConsoleWindow();
	EnableWindow(hwndConsole, TRUE);
}

// todo
// mpnp noise
// apnp

int main() {
	Storage::initialize();

	shared_ptr<Node> node = make_shared<Node>();

	if (node->is_root) {
		node->shell->invoke();

		cin.get();

		node->shell->terminate();
	}
	else {
		string command;
		while (command != "exit") {
			if (node->connections.size() > 0) {
				getline(cin, command);
				disableConsoleInput();
				node->connections[0]->writeData("pnp\nrelay request 0 1 12321\nmpnp\ncommand\n" + command);
				this_thread::sleep_for(chrono::seconds(1));
				enableConsoleInput();
			}
		}
	}

	return 0;
}