#ifndef LOBBYPIECE_H
#define LOBBYPIECE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "lobbytile.h"
#include "lobbyplayer.h"
#include "boardsettings.h"

using namespace std;

class LobbyTile;

class LobbyPiece {
public:
    LobbyTile* tile;
    int color;
    sf::Texture texture;
    sf::Sprite sprite;
    LobbyPlayer* lobby_player;

    LobbyPiece();
    LobbyPiece(LobbyPlayer* lp, string p, int c);
    void createSprite(string piece);
    void moveTo(LobbyTile* destination);
};

#endif