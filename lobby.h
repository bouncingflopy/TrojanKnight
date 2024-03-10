#ifndef LOBBY_H
#define LOBBY_H

#include <SFML/Graphics.hpp>
#include <regex>

#include "node.h"
#include "tiles.h"
#include "player.h"
#include "lobbyplayer.h"
#include "boardsettings.h"
#include "resourceloader.h"

using namespace std;

class Node;
class Tile;
class Player;
class LobbyPlayer;
class LobbyButton;
class LobbyInput;

class Lobby {
public:
	Node* node;
	Tile*** tiles;
	Tile* selected_tile;
	Player* player;
	DHT dht;
	vector<LobbyPlayer*> lobby_players;
	vector<LobbyPlayer*> filtered_lobby_players;
	vector<LobbyPlayer*> page_lobby_players;
	int page = 0;
	bool previous;
	bool next;
	sf::Texture previous_texture;
	sf::Texture next_texture;
	sf::Sprite previous_sprite;
	sf::Sprite next_sprite;
	vector<LobbyButton*> buttons;
	vector<LobbyInput*> inputs;
	LobbyInput* active_input = nullptr;

	Lobby(Node* n);
	void createArrows();
	Tile*** createTiles();
	void draw(sf::RenderWindow& window);
	void updateDHT();
	void applyFilter(string expression);
	void openPage(int new_page);
	void selectTile(Tile* tile);
	void deselectTile(Tile* tile);
	void handlePress(int x, int y);
};

class LobbyButton {
public:
	Lobby* lobby;
	sf::RectangleShape rect;
	vector<sf::Font*> fonts;
	vector<sf::Text*> texts;
	sf::Texture texture;
	sf::Sprite sprite;
	bool has_text;

	LobbyButton(Lobby* l);
	void createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color);
	void createText(string f, int char_size, string t, sf::Vector2f position_offset, sf::Color color, bool bold);
	void createSprite(string file);
	bool checkClick(int x, int y);
	virtual void click();
};

class LobbyButtonStart : public LobbyButton{
public:
	LobbyButtonStart(Lobby* l);
	void changeSubtext(string subtext);
	void click();
};

class LobbyButtonEdit : public LobbyButton {
public:
	LobbyButtonEdit(Lobby* l);
	void click();
};

class LobbyInput {
public:
	Lobby* lobby;
	sf::RectangleShape rect;
	sf::Font font;
	sf::Text text;
	string placeholder;
	bool centered;

	LobbyInput(Lobby* l, string p, bool c);
	void createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color);
	void createText(string f, int char_size, sf::Color color, bool bold);
	bool checkClick(int x, int y);
	void changeText(string new_text);
	void focus();
	void unfocus();
};

class LobbySearch : public LobbyInput {
public:
	LobbySearch(Lobby* l);
};

class LobbyName : public LobbyInput {
public:
	LobbyName(Lobby* l);
};

#endif