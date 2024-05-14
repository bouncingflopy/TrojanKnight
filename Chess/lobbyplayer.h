#ifndef LOBBYPLAYER_H
#define LOBBYPLAYER_H

#include <string>

#include "lobbypiece.h"
#include "lobbytile.h"
#include "boardsettings.h"

using namespace std;

class LobbyPiece;
class LobbyTile;

class LobbyPlayer {
public:
	LobbyPiece* piece;
	int id;
	string name;
	sf::RectangleShape rect;
	sf::Font font;
	sf::Text text;

	LobbyPlayer(int i, string n);
	void moveTo(LobbyTile* tile);
};

#endif