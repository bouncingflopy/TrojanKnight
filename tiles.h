#ifndef TILES_HPP
#define TILES_HPP

#include <SFML/Graphics.hpp>

class Piece;
enum PieceType;

class Tile {
public:
    int row;
    int column;
    int base_color;
    sf::Color color;
    sf::RectangleShape rect;
    bool selected = false;
    bool recentHighlight = false;
    bool checked = false;
    Piece* piece;
    bool promotion_tile = false;
    int me;

    Tile();
    Tile(int r, int c, int m);
    void select();
    void deselect();
    void highlightMove();
    void unhighlightMove();
    void highlightRecent();
    void unhighlightRecent();
    void highlightCheck();
    void unhighlightCheck();
    void highlightCheckmate();
};

class PromotionTile : public Tile {
public:
    PieceType piece_type;
    Tile* base_tile;

    PromotionTile();
    PromotionTile(int r, int c, Tile* b);
};

#endif