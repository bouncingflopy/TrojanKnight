#ifndef LOBBY_H
#define LOBBY_H

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cctype>
#include <string>

#include "node.h"
#include "lobbyplayer.h"
#include "boardsettings.h"
#include "resourceloader.h"

using namespace std;

class Node;
class LobbyTile;
class LobbyPlayer;
class LobbyButton;
class LobbyInput;
class LobbySearch;
class LobbyName;
class LobbyButtonStart;
class LobbyButtonEdit;
class LobbyButtonInvite;
class LobbyInvite;

class Lobby {
public:
	Node* node;
	LobbyTile*** tiles;
	bool running = true;
	LobbyTile* selected_tile;
	DHT dht;
	vector<LobbyPlayer*> lobby_players;
	vector<LobbyPlayer*> filtered_lobby_players;
	vector<LobbyPlayer*> page_lobby_players;
	int page = 0;
	bool previous;
	bool next;
	string filter = "";
	sf::Texture previous_texture;
	sf::Texture next_texture;
	sf::Sprite previous_sprite;
	sf::Sprite next_sprite;
	LobbySearch* input_search;
	LobbyName* input_name;
	LobbyButtonStart* button_start;
	LobbyButtonEdit* button_edit;
	LobbyInput* active_input = nullptr;
	shared_ptr<ChessInvite> incoming_invite;
	shared_ptr<LobbyInvite> lobby_invite;

	Lobby(Node* n);
	void createArrows();
	LobbyTile*** createTiles();
	void draw(sf::RenderWindow& window);
	void update();
	void applyFilter(string expression);
	void openPage(int new_page);
	void displayInvite();
	void acceptInvite();
	void rejectInvite();
	void selectTile(LobbyTile* tile);
	void deselectTile(LobbyTile* tile);
	void handlePress(int x, int y);
};

class LobbyButton {
public:
	Lobby* lobby;
	sf::RectangleShape rect;
	bool activated = false;

	LobbyButton();
	LobbyButton(Lobby* l);
	void createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color);
	bool checkClick(int x, int y);
	virtual void activate();
	virtual void deactivate();
	virtual void click();
};

class LobbyButtonInvite : public LobbyButton {
public:
	char action;
	sf::Texture texture;
	sf::Sprite sprite;

	LobbyButtonInvite();
	LobbyButtonInvite(Lobby* l, char c);
	void initV();
	void initX();
	void createSprite(string file);
	void click();
};

class LobbyButtonStart : public LobbyButton {
public:
	sf::Font font;
	sf::Text text;
	sf::Text subtext;
	bool clickable = false;

	LobbyButtonStart(Lobby* l);
	void createText(sf::Text& current_text, int font_size, string t, int height_offset, sf::Color color, bool bold);
	void changeText(string new_text);
	void changeSubtext(string new_subtext);
	void makeUnclickable();
	void makeClickable();
	void activate();
	void deactivate();
	void click();
};

class LobbyButtonEdit : public LobbyButton {
public:
	LobbyName* input;
	sf::Texture texture;
	sf::Sprite sprite;

	LobbyButtonEdit(Lobby* l, LobbyInput* i);
	void createSprite(string file);
	void activate();
	void deactivate();
	void click();
};

class LobbyInput {
public:
	Lobby* lobby;
	sf::RectangleShape rect;
	sf::Font font;
	sf::Text text;
	bool centered;
	string value;
	bool focused;
	int max_length = MAX_NAME_LENGTH;
	int font_size = 12;
	int max_width = 100;
	float height_offset = 1;

	LobbyInput(Lobby* l, bool c);
	void createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color);
	void createText(string f, sf::Color color, bool bold);
	bool checkClick(int x, int y);
	void displayText(string display);
	virtual void handleEnter();
	void handleUnicode(uint32_t unicode);
	virtual void changedText();
	void focus();
	virtual void unfocus();
};

class LobbySearch : public LobbyInput {
public:
	string placeholder;

	LobbySearch(Lobby* l);
	void changedText();
	void unfocus();
};

class LobbyName : public LobbyInput {
public:
	LobbyButtonEdit* edit;
	string name;

	LobbyName(Lobby* l);
	void handleEnter();
	void unfocus();
};

class LobbyInvite {
public:
	Lobby* lobby;
	sf::RectangleShape rect;
	sf::Font font;
	sf::Text text;
	sf::Text subtext;
	LobbyButtonInvite* button_v;
	LobbyButtonInvite* button_x;

	LobbyInvite(Lobby* l, string name);
	void createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color);
	void createText(sf::Text& current_text, int font_size, string t, sf::Vector2f position_offset, int max_width, sf::Color color, bool bold);
};

#endif