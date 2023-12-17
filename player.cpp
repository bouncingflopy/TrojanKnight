#include "player.h"
#include "sidebar.h"
#include "pieces.h"
#include "boardsettings.h"

Player::Player(int c, string n, int m) : color(c), name(n) {
    name_object = new Name(n, color, m);
}

void Player::setKing(Piece* k) {
    king_piece = k;
}