#include "boardsettings.h"
#include "pieces.h"
#include "tiles.h"
#include "player.h"
#include "resourceloader.h"

PromotionPiece::PromotionPiece(PromotionTile* t, Player* p) {
    tile = t;
    owner = p;
    color = owner->color;
    tile->piece = this;

    me = tile->me;
}

void PromotionPiece::createSprite(string piece) {
    texture = *resource::loadImage(piece + to_string(color));
    texture.setSmooth(true);
    sprite.setTexture(texture);
    sprite.setPosition(sf::Vector2f((tile->base_tile->column * TILESIZE) + (TILESIZE / 2) * tile->column, (((me == 1) ? tile->base_tile->row : 7 - tile->base_tile->row) * TILESIZE) + (TILESIZE / 2) * tile->row));
    float scale = 1.f * (TILESIZE / 2) / texture.getSize().x;
    sprite.setScale(sf::Vector2f(scale, scale));
}

PromotionKnight::PromotionKnight(PromotionTile* t, Player* p) : PromotionPiece(t, p) {
    createSprite("n");
    type = knight;
    tile->piece_type = type;
}

PromotionBishop::PromotionBishop(PromotionTile* t, Player* p) : PromotionPiece(t, p) {
    createSprite("b");
    type = bishop;
    tile->piece_type = type;
}


PromotionRook::PromotionRook(PromotionTile* t, Player* p) : PromotionPiece(t, p) {
    createSprite("r");
    type = rook;
    tile->piece_type = type;
}

PromotionQueen::PromotionQueen(PromotionTile* t, Player* p) : PromotionPiece(t, p) {
    createSprite("q");
    type = queen;
    tile->piece_type = type;
}