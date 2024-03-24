#include <SFML/Graphics.hpp>
#include <iostream>

#include "node.h"
#include "board.h"
#include "lobby.h"
#include "dhtdisplay.h" // only for displaying dht

void lobbyLoop(sf::RenderWindow& window, Lobby*& lobby, bool& press) {
	sf::Event event;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed) window.close();
		else if (event.type == sf::Event::MouseButtonPressed) {
			if (!press && event.mouseButton.button == sf::Mouse::Left) {
				press = true; // add press cooldown for invite spam
				lobby->handlePress(event.mouseButton.x, event.mouseButton.y);
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased) {
			if (press && event.mouseButton.button == sf::Mouse::Left) {
				press = false;
			}
		}
		else if (event.type == sf::Event::TextEntered) {
			if (lobby->active_input) lobby->active_input->handleUnicode(event.text.unicode);
		}
		else if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Enter) {
				if (lobby->active_input) lobby->active_input->handleUnicode('\n');
			}
		}
	}

	lobby->update();
	window.clear(BACKGROUND_COLOR);
	lobby->draw(window);
	window.display();
}

void waitingLoop(sf::RenderWindow& window) {
	sf::Event event;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed) window.close();
	}

	window.clear(BACKGROUND_COLOR);
	window.display();
}

void boardLoop(sf::RenderWindow& window, shared_ptr<Board>& board, bool& press) {
	board->updateClocks();

	sf::Event event;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed) window.close();

		if (!press && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
			press = true;
			board->handlePress(event.mouseButton.x, event.mouseButton.y);
		}

		if (press && event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
			press = false;
		}
	}

	window.clear(BACKGROUND_COLOR);
	board->draw(window);
	window.display();
}

int main() {
	Node* node = new Node();

	thread dht_display_thread([node]() {
		dhtDisplay(node->dht);
		});
	dht_display_thread.detach();

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Chess", sf::Style::Titlebar | sf::Style::Close);
	Lobby* lobby = new Lobby(node);
	
	bool press = false;
	while (window.isOpen()) {
		while (window.isOpen() && lobby->running) {
			lobbyLoop(window, lobby, press);
		}
		press = false;

		while (window.isOpen() && !node->chess_connection) {
			waitingLoop(window);
		}

		shared_ptr<Board> board = node->chess_connection->board;
		while (window.isOpen() && board->running) {
			boardLoop(window, board, press);
		}
		press = false;
	}

	return 0;
}