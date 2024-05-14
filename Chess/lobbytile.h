#ifndef LOBBYTILE_H
#define LOBBYTILE_H

#include <SFML/Graphics.hpp>

#include "lobbypiece.h"
#include "lobbyplayer.h"
#include "boardsettings.h"

using namespace std;

class LobbyPiece;
class LobbyPlayer;

class LobbyTile {
public:
    int row;
    int column;
    int base_color;
    sf::Color color;
    sf::RectangleShape rect;
    bool selected = false;
    LobbyPlayer* lobby_player;

    LobbyTile();
    LobbyTile(int r, int c);
    void select();
    void deselect();
};

#endif