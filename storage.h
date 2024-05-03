#ifndef STORAGE_H
#define STORAGE_H

#include <string>

using namespace std;

namespace Storage {
	static string appdata;
	const string folder = "cpnp";
	const string keys = "keys";
	const string name = "name.txt";

	void loadAppdata();
	string getKey(int id);
	void setKey(int id, string data);
	string getName();
	void setName(string data);
	void initialize();
}

#endif