#include <SFML/Graphics.hpp>
#include <iostream>

#include "node.h"
#include "board.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Chess", sf::Style::Titlebar | sf::Style::Close);
    Board* board = new Board(1);

    bool press = false;
    while (window.isOpen())
    {
        board->updateClocks();

        /*if (connection->incoming_messages.size() > 0) {
            board->decodeCPNP(connection->incoming_messages.front());
            connection->incoming_messages.pop();
        }

        if (board->message_waiting) {
            connection->writeData(board->popMessage());
        }*/
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();

            if (board->enabled) {
                if (!press && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    press = true;
                    board->handlePress(event.mouseButton.x, event.mouseButton.y);
                }

                if (press && event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                    press = false;
                }
            }
        }

        window.clear(BACKGROUND_COLOR);
        board->draw(window);
        window.display();
    }

    return 0;
}

//int main() {
//    Node* node = new Node();
//
//    thread input_thread([&node]() {
//        string input;
//        while (1) {
//            cin >> input;
//            for (int i = 0; i < node->connections.size(); i++) {
//                node->connections[i]->writeData(input);
//            }
//        }
//    });
//    
//    while (1) {
//        for (int i = 0; i < node->connections.size(); i++) {
//            if (node->connections[i]->incoming_messages.size() > 0) {
//                cout << node->connections[i]->incoming_messages.front();
//                node->connections[i]->incoming_messages.pop();
//            }
//        }
//    }
//
//    return 0;
//}