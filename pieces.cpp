#include "boardsettings.h"
#include "pieces.h"
#include "player.h"
#include "tiles.h"
#include "moves.h"
#include "resourceloader.h"

Piece::Piece() {}

Piece::Piece(Tile* t, Player* p) {
    tile = t;
    owner = p;
    color = owner->color;
    tile->piece = this;

    me = tile->me;
}

void Piece::createSprite(string piece) {
    texture = *resource::loadImage(piece + to_string(color));
    texture.setSmooth(true);
    sprite.setTexture(texture);
    sprite.setPosition(sf::Vector2f(TILESIZE * tile->column, TILESIZE * ((me == 1) ? tile->row : 7 - tile->row)));
    float scale = 1.f * TILESIZE / texture.getSize().x;
    sprite.setScale(sf::Vector2f(scale, scale));
}

Move* Piece::getMove(Tile* destination) {
    for (int i = 0; i < moves.size(); i++) {
        if (moves[i]->destination == destination) return moves[i];
    }

    return NULL;
}

void Piece::calculateAvailableMoves(Tile*** tiles) {}

void Piece::calculateAvailableMoves(Tile*** tiles, Move* last_move) {
    calculateAvailableMoves(tiles);
}

void Piece::highlightMoves() {
    for (int i = 0; i < moves.size(); i++) {
        moves[i]->destination->highlightMove();
    }
}

void Piece::unhighlightMoves() {
    for (int i = 0; i < moves.size(); i++) {
        moves[i]->destination->unhighlightMove();
    }
}

void Piece::moveTo(Tile* destination, int time_direction) {
    tile->piece = NULL;

    tile = destination;
    tile->piece = this;
    sprite.setPosition(sf::Vector2f(TILESIZE * tile->column, TILESIZE * ((me == 1) ? tile->row : 7 - tile->row)));

    times_moved += time_direction;
}

Pawn::Pawn(Tile* t, Player* p) : Piece(t, p) {
    createSprite("p");
    type = pawn;
}

Move* Pawn::getMove(Tile* destination, PieceType piece_type) {
    for (int i = 0; i < moves.size(); i++) {
        if (moves[i]->destination == destination && ((MovePromotion*)moves[i])->piece_type == piece_type) return moves[i];
    }

    return NULL;
}

Rook::Rook(Tile* t, Player* p) : Piece(t, p) {
    createSprite("r");
    type = rook;
}


Knight::Knight(Tile* t, Player* p) : Piece(t, p) {
    createSprite("n");
    type = knight;
}

Bishop::Bishop(Tile* t, Player* p) : Piece(t, p) {
    createSprite("b");
    type = bishop;
}

Queen::Queen(Tile* t, Player* p) : Piece(t, p) {
    createSprite("q");
    type = queen;
}

King::King(Tile* t, Player* p) : Piece(t, p) {
    createSprite("k");
    type = king;
    owner->king_piece = this;
}