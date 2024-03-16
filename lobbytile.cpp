#include "lobbytile.h"
#include "boardsettings.h"

LobbyTile::LobbyTile() {}

LobbyTile::LobbyTile(int r, int c) : row(r), column(c) {
    base_color = (row % 2 == column % 2) ? 0 : 1;
    color = (base_color == 0) ? TILE_BLACK : TILE_WHITE;

    rect.setSize(sf::Vector2f(TILESIZE, TILESIZE));
    rect.setPosition(sf::Vector2f(TILESIZE * column, TILESIZE * row));
    rect.setFillColor(color);
}

void LobbyTile::select() {
    selected = true;
    color = (base_color == 0) ? TILE_BLACK_SELECT : TILE_WHITE_SELECT;
    rect.setFillColor(color);
}

void LobbyTile::deselect() {
    selected = false;
    color = (base_color == 0) ? TILE_BLACK : TILE_WHITE;
    rect.setFillColor(color);
}