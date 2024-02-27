#ifndef PIECES_HPP
#define PIECES_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "boardsettings.h"

class Tile;
class Move;
class Player;
enum PieceType;
class PromotionTile;

class Piece {
public:
    Tile* tile;
    int color;
    sf::Texture texture;
    sf::Sprite sprite;
    vector<Move*> moves;
    Player* owner;
    int times_moved = 0;
    PieceType type;
    int me;

    Piece();
    Piece(Tile* t, Player* p);
    virtual void createSprite(string piece);
    Move* getMove(Tile* destination);
    virtual void calculateAvailableMoves(Tile*** tiles);
    virtual void calculateAvailableMoves(Tile*** tiles, Move* last_move);
    void highlightMoves();
    void unhighlightMoves();
    void moveTo(Tile* destination, int time_direction);
};

class Pawn : public Piece {
public:
    PieceType promotion_types[4] = { knight, bishop, rook, queen };

    Pawn();
    Pawn(Tile* t, Player* p);
    void calculateAvailableMoves(Tile*** tiles, Move* last_move);
    Move* getMove(Tile* destination, PieceType piece_type);
};

class Rook : public Piece {
public:
    Rook();
    Rook(Tile* t, Player* p);
    void calculateAvailableMoves(Tile*** tiles);
};

class Knight : public Piece {
public:
    Knight();
    Knight(Tile* t, Player* p);
    void calculateAvailableMoves(Tile*** tiles);
};

class Bishop : public Piece {
public:
    Bishop();
    Bishop(Tile* t, Player* p);
    void calculateAvailableMoves(Tile*** tiles);
};

class Queen : public Piece {
public:
    Queen();
    Queen(Tile* t, Player* p);
    void calculateAvailableMoves(Tile*** tiles);
};

class King : public Piece {
public:
    King();
    King(Tile* t, Player* p);
    void calculateAvailableMoves(Tile*** tiles);
};

class PromotionPiece : public Piece {
public:
    PromotionTile* tile;

    PromotionPiece();
    PromotionPiece(PromotionTile* t, Player* p);
    void createSprite(string piece);
};

class PromotionKnight : public PromotionPiece {
public:
    PromotionKnight();
    PromotionKnight(PromotionTile* t, Player* p);
};

class PromotionBishop : public PromotionPiece {
public:
    PromotionBishop();
    PromotionBishop(PromotionTile* t, Player* p);
};

class PromotionRook : public PromotionPiece {
public:
    PromotionRook();
    PromotionRook(PromotionTile* t, Player* p);
};

class PromotionQueen : public PromotionPiece {
public:
    PromotionQueen();
    PromotionQueen(PromotionTile* t, Player* p);
};

#endif