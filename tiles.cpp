#include "boardsettings.h"
#include "tiles.h"
#include "pieces.h"

Tile::Tile() {}

Tile::Tile(int r, int c, int m) : row(r), column(c), me(m) {
    base_color = (row % 2 == column % 2) ? 0 : 1;
    color = (base_color == 0) ? TILE_BLACK : TILE_WHITE;

    rect.setSize(sf::Vector2f(TILESIZE, TILESIZE));
    rect.setPosition(sf::Vector2f(TILESIZE * column, TILESIZE * ((me == 1) ? row : 7 - row)));
    rect.setFillColor(color);
}

void Tile::select() {
    selected = true;
    color = (base_color == 0) ? TILE_BLACK_SELECT : TILE_WHITE_SELECT;
    rect.setFillColor(color);
}

void Tile::deselect() {
    selected = false;
    if (recentHighlight) {
        highlightRecent();
    }
    else if (checked) {
        highlightCheck();
    }
    else {
        color = (base_color == 0) ? TILE_BLACK : TILE_WHITE;
    }
    rect.setFillColor(color);
}

void Tile::highlightMove() {
    color = (base_color == 0) ? TILE_BLACK_MOVEABLE : TILE_WHITE_MOVEABLE;
    rect.setFillColor(color);
}

void Tile::unhighlightMove() {
    if (recentHighlight) {
        highlightRecent();
    }
    else if (checked) {
        highlightCheck();
    }
    else {
        color = (base_color == 0) ? TILE_BLACK : TILE_WHITE;
    }
    rect.setFillColor(color);
}

void Tile::highlightRecent() {
    recentHighlight = true;
    color = (base_color == 0) ? TILE_BLACK_RECENT : TILE_WHITE_RECENT;
    rect.setFillColor(color);
}

void Tile::unhighlightRecent() {
    recentHighlight = false;
    color = (base_color == 0) ? TILE_BLACK : TILE_WHITE;
    rect.setFillColor(color);
}

void Tile::highlightCheck() {
    checked = true;
    color = (base_color == 0) ? TILE_BLACK_CHECK : TILE_WHITE_CHECK;
    rect.setFillColor(color);
}

void Tile::unhighlightCheck() {
    checked = false;
    color = (base_color == 0) ? TILE_BLACK : TILE_WHITE;
    rect.setFillColor(color);
}

void Tile::highlightCheckmate() {
    color = TILE_CHECKMATE;
    rect.setFillColor(color);
}

PromotionTile::PromotionTile(int r, int c, Tile* b) {
    row = r;
    column = c;
    base_tile = b;

    me = base_tile->me;

    base_color = (row % 2 == column % 2) ? 0 : 1;
    color = (base_color == 0) ? TILE_BLACK_MOVEABLE : TILE_WHITE_MOVEABLE;

    rect.setSize(sf::Vector2f(TILESIZE / 2, TILESIZE / 2));
    rect.setPosition(sf::Vector2f((base_tile->column * TILESIZE) + (TILESIZE / 2) * column, (((me == 1) ? base_tile->row : 7 - base_tile->row) * TILESIZE) + (TILESIZE / 2) * row));
    rect.setFillColor(color);

    promotion_tile = true;
}