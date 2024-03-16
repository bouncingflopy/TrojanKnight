#include "lobbypiece.h"
#include "resourceloader.h"

LobbyPiece::LobbyPiece() {}

LobbyPiece::LobbyPiece(LobbyPlayer* lp, string p, int c) : lobby_player(lp), color(c) {
    createSprite(p);
}

void LobbyPiece::createSprite(string piece) {
    texture = *resource::loadImage(piece + to_string(color));
    texture.setSmooth(true);
    sprite.setTexture(texture);
    float scale = 1.f * TILESIZE / texture.getSize().x;
    sprite.setScale(sf::Vector2f(scale, scale));
}

void LobbyPiece::moveTo(LobbyTile* destination) {
    if (tile) tile->lobby_player = nullptr;
    tile = destination;
    tile->lobby_player = lobby_player;

    sprite.setPosition(sf::Vector2f(TILESIZE * tile->column, TILESIZE * tile->row));
}