#include <SFML/Graphics.hpp>
#include <iostream>

#include "node.h"
#include "board.h"
#include "lobby.h"
#include "launcher.h"
#include "storage.h"
#include "dhtdisplay.h" // only for displaying dht

void resize(sf::RenderWindow& window) {
	sf::Vector2u size = window.getSize();

	float aspect_ratio = 1.f * WINDOW_WIDTH / WINDOW_HEIGHT;

	if (size.y * aspect_ratio <= size.x)
	{
		size.x = size.y * aspect_ratio;
	}
	else {
		size.y = size.x * 1 / aspect_ratio;
	}

	window.setSize(size);
}

sf::Vector2u getCoordinates(sf::RenderWindow& window, int x, int y) {
	sf::Vector2u coordinates;

	float scale = 1.f * WINDOW_WIDTH / window.getSize().x;

	coordinates.x = x * scale;
	coordinates.y = y * scale;

	return coordinates;
}

void lobbyLoop(sf::RenderWindow& window, Lobby*& lobby, bool& press) {
	sf::Event event;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed) window.close();
		else if (event.type == sf::Event::Resized) resize(window);

		if (event.type == sf::Event::MouseButtonPressed) {
			if (!press && event.mouseButton.button == sf::Mouse::Left) {
				press = true; // add press cooldown for invite spam
				sf::Vector2u coordinates = getCoordinates(window, event.mouseButton.x, event.mouseButton.y);
				lobby->handlePress(coordinates.x, coordinates.y);
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
		else if (event.type == sf::Event::Resized) resize(window);

		if (!press && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
			press = true;
			sf::Vector2u coordinates = getCoordinates(window, event.mouseButton.x, event.mouseButton.y);
			board->handlePress(coordinates.x, coordinates.y);
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
	Storage::initialize();

	if (!Launcher::checkPrivileges()) { // not needed?
		cout << "must run in admin mode!" << endl;

		cin.get();
		return 0;
	}

	Launcher::main();

	Node* node = new Node();

	thread dht_display_thread([node]() {
		if (node->is_root) dhtDisplay(node->dht); // lan
		});
	dht_display_thread.detach();

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Chess", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
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

		shared_ptr<Board> board;
		if (window.isOpen()) board = node->chess_connection->board;
		while (window.isOpen() && board->running) {
			boardLoop(window, board, press);
		}
		press = false;

		lobby->running = true;
	}

	return 0;
}