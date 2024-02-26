#include <SFML/Graphics.hpp>
#include <iostream>

#include "node.h"
#include "board.h"

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

void printConnections(Node* node, int i) {
    cout << "-------------------------------------------\nCONNECTIONS " << to_string(i) << endl;
    for (Connection* connection : node->connections) {
        cout << "connection: " << connection->ip << "::" << connection->port << ", connected? " << (connection->connected ? "yes" : "no") << endl;
    }
    cout << "-------------------------------------------" << endl;
}

void printDHT(Node* node, int i) {
    cout << "**********************************************\nDHT " << to_string(i) << endl;
    cout << node->dht.toString();
    cout << "**********************************************" << endl;
}

void createNode(vector<Node*> &nodes) {
    nodes.push_back(new Node());
}

int main() { // initialize board with connection from node that is the chess connection
    cout << "open wireshark" << endl;
    // ip.src == 127.0.0.1 && ip.addr == 127.0.0.1 && (udp || icmp) && data != "keepalive" && data != "syn" && data != "ack"

    vector<Node*> nodes;
    string input;

    while (true) {
        cin >> input;
        if (input == "new") {
            createNode(nodes);
            continue;
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