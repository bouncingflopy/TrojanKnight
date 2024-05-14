#ifndef STORAGE_H
#define STORAGE_H

#include <string>

using namespace std;

namespace Storage {
	static string appdata;
	const string folder = "apnp";
	const string keys = "keys";

	void loadAppdata();
	string getKey(int id);
	void setKey(int id, string data);
	string getName();
	string getShellOutput();
	void initialize();
}

#endif