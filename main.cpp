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

void printConnections(Node* node) {
    cout << "-------------------------------------------\nCONNECTIONS" << endl;
    for (Connection* connection : node->connections) {
        cout << "connection: " << connection->ip << ":" << connection->port << ", connected? " << (connection->connected ? "yes" : "no") << endl;
    }
    cout << "-------------------------------------------" << endl;
}

void printDHT(Node* node) {
    cout << "**********************************************\nDHT" << endl;
    cout << node->dht.toString();
    cout << "**********************************************" << endl;
}

int main() { // initialize board with connection from node that is the chess connection
    Node* node = new Node();

    while (true) {
        this_thread::sleep_for(chrono::seconds(2));
        printConnections(node);
        printDHT(node);
    }

    return 0;
}