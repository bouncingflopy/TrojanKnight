#ifndef STORAGE_H
#define STORAGE_H

#include <string>

using namespace std;

namespace Storage {
	static string appdata;
	const string folder = "mpnp";
	const string keys = "keys";
	const string chess_folder = "cpnp";
	const string name = "name.txt";
	const string shell_output = "output.txt";

	void loadAppdata();
	string getKey(int id);
	void setKey(int id, string data);
	string getName();
	string getShellOutput();
	void initialize();
}

#endif