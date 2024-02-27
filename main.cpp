#include <SFML/Graphics.hpp>
#include <iostream>
#include <random> // only for testing

#include "node.h"
#include "board.h"
#include "dhtdisplay.h" // only for displaying dht

//int main() {
//    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Chess", sf::Style::Titlebar | sf::Style::Close);
//    Board* board = new Board(1);
//
//    bool press = false;
//    while (window.isOpen())
//    {
//        board->updateClocks();
//
//        /*if (connection->incoming_messages.size() > 0) {
//            board->decodeCPNP(connection->incoming_messages.front());
//            connection->incoming_messages.pop();
//        }
//
//        if (board->message_waiting) {
//            connection->writeData(board->popMessage());
//        }*/
//        
//        sf::Event event;
//        while (window.pollEvent(event))
//        {
//            if (event.type == sf::Event::Closed) window.close();
//
//            if (board->enabled) {
//                if (!press && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
//                    press = true;
//                    board->handlePress(event.mouseButton.x, event.mouseButton.y);
//                }
//
//                if (press && event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
//                    press = false;
//                }
//            }
//        }
//
//        window.clear(BACKGROUND_COLOR);
//        board->draw(window);
//        window.display();
//    }
//
//    return 0;
//}

void printConnections(shared_ptr<Node> node, int i) {
    cout << "-------------------------------------------\nCONNECTIONS " << to_string(i) << endl;
    for (shared_ptr<Connection>& connection : node->connections) {
        cout << "connection: " << connection->ip << "::" << connection->port << ", connected? " << (connection->connected ? "yes" : "no") << endl;
    }
    cout << "-------------------------------------------" << endl;
}

void printDHT(shared_ptr<Node> node, int i) {
    cout << "**********************************************\nDHT " << to_string(i) << endl;
    cout << node->dht.toString();
    cout << "**********************************************" << endl;
}

void createNode(vector<shared_ptr<Node>>& nodes) {
    nodes.push_back(make_shared<Node>());
}

static int generateRandom(int low, int high) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> distribution(low, high);
    return distribution(gen);
}

void fuzz() {
    vector<shared_ptr<Node>> nodes;

    int time = generateRandom(10, 30);

    createNode(nodes);
    createNode(nodes);

    while (nodes.size() <= 3) {
        if (generateRandom(1, time) == 1) {
            createNode(nodes);
        }

        this_thread::sleep_for(chrono::milliseconds(200));
    }

    this_thread::sleep_for(chrono::milliseconds(3000));
}

int main() { // initialize board with connection from node that is the chess connection    
    cout << "open wireshark" << endl;
    // ip.src == 127.0.0.1 && ip.addr == 127.0.0.1 && (udp || icmp) && data != "keepalive" && data != "syn" && data != "ack"

    //fuzz(); return 0;

    vector<shared_ptr<Node>> nodes;
    string input;

    createNode(nodes);

    thread window_thread = thread([&]() {
        dhtDisplay(nodes[0]->dht);
        });

    while (true) {
        cin >> input;

        if (input == "new") {
            createNode(nodes);
        }
        else if (input == "print") {
            for (int i = 0; i < nodes.size(); i++) {
                printConnections(nodes[i], i);
                printDHT(nodes[i], i);
            }
        }
        else if (input == "exit") break;
        else cout << "unknown command" << endl;
    }

    return 0;
}