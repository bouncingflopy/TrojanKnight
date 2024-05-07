#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <Windows.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include <thread>

using namespace std;

BOOL isElevated() {
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);

		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			fRet = Elevation.TokenIsElevated;
		}
	}

	if (hToken) {
		CloseHandle(hToken);
	}

	return fRet;
}

//int main() {
//	ofstream file("D://Users//flopybouncer//Desktop//text.txt", ios_base::app);
//
//	time_t current_time = time(nullptr);
//	char time_string[100];
//	strftime(time_string, sizeof(time_string), "%H:%M:%S", localtime(&current_time));
//
//	if (isElevated()) file << time_string << "> elevated" << endl;
//	else file << time_string << "> not elevated" << endl;
//	
//	file.close();
//
//	while (true) {
//		this_thread::sleep_for(chrono::seconds(5));
//	}
//
//	return 0;
//}