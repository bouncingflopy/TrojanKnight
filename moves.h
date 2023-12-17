#ifndef MOVES_HPP
#define MOVES_HPP

class Tile;
class Piece;
enum PieceType;

class Move {
public:
    Tile* source;
    Piece* piece;
    Tile* destination;
    Piece* eat;
    Tile* eat_tile;
    bool promotion = false;
    bool castle = false;

    Move();
    Move(Tile* s, Piece* p, Tile* d);
    Move(Tile* s, Piece* p, Tile* d, Piece* e);
    Move(Tile* s, Piece* p, Tile* d, Piece* e, Tile* et);
    void highlight();
    void unhighlight();
};

class MoveCastle : public Move {
public:
    Tile* source_rook;
    Piece* piece_rook;
    Tile* destination_rook;

    MoveCastle();
    MoveCastle(Tile* sk, Tile* sr, Piece* k, Piece* r, Tile* dk, Tile* dr);
};

class MovePromotion : public Move {
public:
    PieceType piece_type;

    MovePromotion();
    MovePromotion(Tile* s, Piece* p, Tile* d, PieceType pt);
    MovePromotion(Tile* s, Piece* p, Tile* d, Piece* e, PieceType pt);
};

#endif