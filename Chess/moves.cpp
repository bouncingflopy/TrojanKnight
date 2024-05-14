#include "boardsettings.h"
#include "tiles.h"
#include "pieces.h"
#include "moves.h"

Move::Move() {}

Move::Move(Tile* s, Piece* p, Tile* d) {
    source = s;
    piece = p;
    destination = d;
}

Move::Move(Tile* s, Piece* p, Tile* d, Piece* e) {
    source = s;
    piece = p;
    destination = d;
    eat = e;
}

Move::Move(Tile* s, Piece* p, Tile* d, Piece* e, Tile* et) {
    source = s;
    piece = p;
    destination = d;
    eat = e;
    eat_tile = et;
}

void Move::highlight() {
    source->highlightRecent();
    destination->highlightRecent();
}

void Move::unhighlight() {
    source->unhighlightRecent();
    destination->unhighlightRecent();
}

MoveCastle::MoveCastle(Tile* sk, Tile* sr, Piece* k, Piece* r, Tile* dk, Tile* dr) {
    source = sk;
    source_rook = sr;
    piece = k;
    piece_rook = r;
    destination = dk;
    destination_rook = dr;

    castle = true;
}

MovePromotion::MovePromotion(Tile* s, Piece* p, Tile* d, PieceType pt) {
    source = s;
    piece = p;
    destination = d;
    piece_type = pt;

    promotion = true;
}

MovePromotion::MovePromotion(Tile* s, Piece* p, Tile* d, Piece* e, PieceType pt) {
    source = s;
    piece = p;
    destination = d;
    eat = e;
    piece_type = pt;

    promotion = true;
}