#include <SFML/Graphics.hpp>
#include <iostream>

#include "node.h"
#include "board.h"
#include "lobby.h"
#include "dhtdisplay.h" // only for displaying dht

//int main() { // initialize board with connection from node that is the chess connection
//    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Chess", sf::Style::Titlebar | sf::Style::Close);
//    Board* board = new Board(1);
//
//    bool press = false;
//    while (window.isOpen()) {
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
//        while (window.pollEvent(event)) {
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

int main() {
    shared_ptr<Node> node = make_shared<Node>();

    dhtDisplay(node->dht);
    //while (true) { this_thread::sleep_for(chrono::seconds(3)); }

    return 0;
}

//int main() {
//	Node* node = new Node();
//
//	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Chess", sf::Style::Titlebar | sf::Style::Close);
//	Lobby* lobby = new Lobby(node);
//	
//	bool press = false;
//	while (window.isOpen()) {
//	    sf::Event event;
//	    while (window.pollEvent(event)) {
//	        if (event.type == sf::Event::Closed) window.close();
//	        else if (event.type == sf::Event::MouseButtonPressed) {
//				if (!press && event.mouseButton.button == sf::Mouse::Left) {
//					press = true;
//					lobby->handlePress(event.mouseButton.x, event.mouseButton.y);
//				}
//	        }
//			else if (event.type == sf::Event::MouseButtonReleased) {
//				if (press && event.mouseButton.button == sf::Mouse::Left) {
//					press = false;
//				}
//	        }
//			else if (event.type == sf::Event::TextEntered) {
//				if (lobby->active_input) lobby->active_input->handleUnicode(event.text.unicode);
//			}
//			else if (event.type == sf::Event::KeyPressed) {
//				if (event.key.code == sf::Keyboard::Enter) {
//					if (lobby->active_input) lobby->active_input->handleUnicode('\n');
//				}
//			}
//	    }
//		
//		lobby->updateDHT();
//	    window.clear(BACKGROUND_COLOR);
//	    lobby->draw(window);
//	    window.display();
//	}
//
//	return 0;
//}