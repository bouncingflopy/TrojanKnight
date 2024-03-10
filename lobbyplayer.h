#ifndef LOBBYPLAYER_H
#define LOBBYPLAYER_H

#include "pieces.h"
#include "tiles.h"
#include "player.h"

class Piece;
class Tile;

class LobbyPlayer {
public:
	Piece* piece;
	int id;
	string name;
	sf::RectangleShape rect;
	sf::Font font;
	sf::Text text;

	LobbyPlayer(int i, string n);
	void moveTo(Tile* tile, Player* player);
};

#endif