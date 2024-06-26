#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include <cstdlib>
#include <cstring>

#include "storage.h"

using namespace std;
namespace fs = std::experimental::filesystem;

void Storage::loadAppdata() {
    char* appdata_path;
    size_t len;
    errno_t err = _dupenv_s(&appdata_path, &len, "APPDATA");

    if (err != 0) {
        cerr << "Error: Unable to get APPDATA environment variable." << endl;
        return;
    }

    appdata = string(appdata_path);
    free(appdata_path);
}

string Storage::getKey(int id) {
    ifstream file(appdata + "/" + folder + "/" + keys + "/key_" + to_string(id) + ".txt");

    if (file) {
        string key;

        char ch;
        while (file.get(ch)) {
            key += ch;
        }

        file.close();

        return key;
    }
    else {
        return "";
    }
}

void Storage::setKey(int id, string data) {
    ofstream file(appdata + "/" + folder + "/" + keys + "/key_" + to_string(id) + ".txt");

    if (file) {
        file << data;
        file.close();
    }
    else {
        cerr << "Error: Unable to create key file." << endl;
    }
}

string Storage::getName() {
    ifstream file(appdata  + "/" + chess_folder + "/" + name);

    if (file) {
        string name;

        getline(file, name);
        file.close();

        return name;
    }
    else {
        return "";
    }
}

string Storage::getShellOutput() {
    return appdata + "/" + folder + "/" + shell_output;
}

void Storage::initialize() {
    loadAppdata();

    string folder_path = appdata + "/" + folder;
    if (!fs::exists(folder_path)) {
        fs::create_directory(folder_path);
    }

    string keys_path = folder_path + "/" + keys;
    if (!fs::exists(keys_path)) {
        fs::create_directory(keys_path);
    }
    //else {
    //    for (const auto& entry : fs::directory_iterator(keys_path)) {
    //        fs::remove(entry.path());
    //    }
    //} // wan - lan

    string chess_folder_path = appdata + "/" + chess_folder;
    if (!fs::exists(chess_folder_path)) {
        fs::create_directory(chess_folder_path);
    }

    string name_path = chess_folder_path + "/" + name;
    if (!fs::exists(name_path)) {
        ofstream nameFile(name_path);

        if (!nameFile) {
            cerr << "Error: Unable to create name file." << endl;
        }
    }

    string shell_output_path = folder_path + "/" + shell_output;
    ofstream outputFile(shell_output_path, ios::trunc);
    if (!outputFile) {
        cerr << "Error: Unable to create shell output file." << endl;
    }
}