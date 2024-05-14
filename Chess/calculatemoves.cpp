#include "boardsettings.h"
#include "pieces.h"
#include "player.h"
#include "tiles.h"
#include "moves.h"

void Pawn::calculateAvailableMoves(Tile*** tiles, Move* last_move) {
    moves.clear();

    Tile* testing;

    if (times_moved == 0) {
        testing = tiles[tile->row + ((color == 0) ? 2 : -2)][tile->column];
        if (!testing->piece && !tiles[tile->row + ((color == 0) ? 1 : -1)][tile->column]->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
    }

    testing = tiles[tile->row + ((color == 0) ? 1 : -1)][tile->column];
    if (!testing->piece) {
        if (tile->row + ((color == 0) ? 1 : -1) == ((color == 0) ? 7 : 0)) {
            for (int t = 0; t < 4; t++) {
                moves.push_back(new MovePromotion(tile, this, testing, promotion_types[t]));
            }
        }
        else {
            moves.push_back(new Move(tile, this, testing));
        }

    }

    if (tile->column < 7) {
        testing = tiles[tile->row + ((color == 0) ? 1 : -1)][tile->column + 1];
        if (testing->piece && testing->piece->owner != owner) {
            if (tile->row + ((color == 0) ? 1 : -1) == ((color == 0) ? 7 : 0)) {
                for (int t = 0; t < 4; t++) {
                    moves.push_back(new MovePromotion(tile, this, testing, testing->piece, promotion_types[t]));
                }
            }
            else {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }
    }

    if (tile->column > 0) {
        testing = tiles[tile->row + ((color == 0) ? 1 : -1)][tile->column - 1];
        if (testing->piece && testing->piece->owner != owner) {
            if (tile->row + ((color == 0) ? 1 : -1) == ((color == 0) ? 7 : 0)) {
                for (int t = 0; t < 4; t++) {
                    moves.push_back(new MovePromotion(tile, this, testing, testing->piece, promotion_types[t]));
                }
            }
            else {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }
    }

    if (color == 0 && tile->row == 4 && last_move->piece->type == pawn && last_move->source->row == 6 && last_move->destination->row == 4) {
        if (last_move->destination->column == tile->column + 1 || last_move->destination->column == tile->column - 1) {
            moves.push_back(new Move(tile, this, tiles[last_move->destination->row + 1][last_move->destination->column], last_move->piece, last_move->destination));
        }
    }
    if (color == 1 && tile->row == 3 && last_move->piece->type == pawn && last_move->source->row == 1 && last_move->destination->row == 3) {
        if (last_move->destination->column == tile->column + 1 || last_move->destination->column == tile->column - 1) {
            moves.push_back(new Move(tile, this, tiles[last_move->destination->row - 1][last_move->destination->column], last_move->piece, last_move->destination));
        }
    }
}

void Rook::calculateAvailableMoves(Tile*** tiles) {
    moves.clear();

    Tile* testing;

    if (tile->row < 7) {
        testing = tiles[tile->row + 1][tile->column];
        while (!testing->piece && testing->row < 7) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row + 1][testing->column];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row > 0) {
        testing = tiles[tile->row - 1][tile->column];
        while (!testing->piece && testing->row > 0) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row - 1][testing->column];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->column < 7) {
        testing = tiles[tile->row][tile->column + 1];
        while (!testing->piece && testing->column < 7) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row][testing->column + 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->column > 0) {
        testing = tiles[tile->row][tile->column - 1];
        while (!testing->piece && testing->column > 0) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row][testing->column - 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }
}

void Knight::calculateAvailableMoves(Tile*** tiles) {
    moves.clear();

    Tile* testing;

    if (tile->row > 1) {
        if (tile->column > 0) {
            testing = tiles[tile->row - 2][tile->column - 1];
            if (!testing->piece) {
                moves.push_back(new Move(tile, this, testing));
            }
            else if (testing->piece->owner != owner) {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }

        if (tile->column < 7) {
            testing = tiles[tile->row - 2][tile->column + 1];
            if (!testing->piece) {
                moves.push_back(new Move(tile, this, testing));
            }
            else if (testing->piece->owner != owner) {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }
    }

    if (tile->row < 6) {
        if (tile->column > 0) {
            testing = tiles[tile->row + 2][tile->column - 1];
            if (!testing->piece) {
                moves.push_back(new Move(tile, this, testing));
            }
            else if (testing->piece->owner != owner) {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }

        if (tile->column < 7) {
            testing = tiles[tile->row + 2][tile->column + 1];
            if (!testing->piece) {
                moves.push_back(new Move(tile, this, testing));
            }
            else if (testing->piece->owner != owner) {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }
    }

    if (tile->column > 1) {
        if (tile->row > 0) {
            testing = tiles[tile->row - 1][tile->column - 2];
            if (!testing->piece) {
                moves.push_back(new Move(tile, this, testing));
            }
            else if (testing->piece->owner != owner) {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }

        if (tile->row < 7) {
            testing = tiles[tile->row + 1][tile->column - 2];
            if (!testing->piece) {
                moves.push_back(new Move(tile, this, testing));
            }
            else if (testing->piece->owner != owner) {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }
    }

    if (tile->column < 6) {
        if (tile->row > 0) {
            testing = tiles[tile->row - 1][tile->column + 2];
            if (!testing->piece) {
                moves.push_back(new Move(tile, this, testing));
            }
            else if (testing->piece->owner != owner) {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }

        if (tile->row < 7) {
            testing = tiles[tile->row + 1][tile->column + 2];
            if (!testing->piece) {
                moves.push_back(new Move(tile, this, testing));
            }
            else if (testing->piece->owner != owner) {
                moves.push_back(new Move(tile, this, testing, testing->piece));
            }
        }
    }
}

void Bishop::calculateAvailableMoves(Tile*** tiles) {
    moves.clear();

    Tile* testing;

    if (tile->row < 7 && tile->column < 7) {
        testing = tiles[tile->row + 1][tile->column + 1];
        while (!testing->piece && testing->row < 7 && testing->column < 7) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row + 1][testing->column + 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row < 7 && tile->column > 0) {
        testing = tiles[tile->row + 1][tile->column - 1];
        while (!testing->piece && testing->row < 7 && testing->column > 0) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row + 1][testing->column - 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row > 0 && tile->column > 0) {
        testing = tiles[tile->row - 1][tile->column - 1];
        while (!testing->piece && testing->row > 0 && testing->column > 0) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row - 1][testing->column - 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row > 0 && tile->column < 7) {
        testing = tiles[tile->row - 1][tile->column + 1];
        while (!testing->piece && testing->row > 0 && testing->column < 7) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row - 1][testing->column + 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }
}

void Queen::calculateAvailableMoves(Tile*** tiles) {
    moves.clear();

    Tile* testing;

    if (tile->row < 7) {
        testing = tiles[tile->row + 1][tile->column];
        while (!testing->piece && testing->row < 7) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row + 1][testing->column];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row > 0) {
        testing = tiles[tile->row - 1][tile->column];
        while (!testing->piece && testing->row > 0) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row - 1][testing->column];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->column < 7) {
        testing = tiles[tile->row][tile->column + 1];
        while (!testing->piece && testing->column < 7) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row][testing->column + 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->column > 0) {
        testing = tiles[tile->row][tile->column - 1];
        while (!testing->piece && testing->column > 0) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row][testing->column - 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row < 7 && tile->column < 7) {
        testing = tiles[tile->row + 1][tile->column + 1];
        while (!testing->piece && testing->row < 7 && testing->column < 7) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row + 1][testing->column + 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row < 7 && tile->column > 0) {
        testing = tiles[tile->row + 1][tile->column - 1];
        while (!testing->piece && testing->row < 7 && testing->column > 0) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row + 1][testing->column - 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row > 0 && tile->column > 0) {
        testing = tiles[tile->row - 1][tile->column - 1];
        while (!testing->piece && testing->row > 0 && testing->column > 0) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row - 1][testing->column - 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }

    if (tile->row > 0 && tile->column < 7) {
        testing = tiles[tile->row - 1][tile->column + 1];
        while (!testing->piece && testing->row > 0 && testing->column < 7) {
            moves.push_back(new Move(tile, this, testing));
            testing = tiles[testing->row - 1][testing->column + 1];
        }

        if (!testing->piece) {
            moves.push_back(new Move(tile, this, testing));
        }
        else if (testing->piece->owner != owner) {
            moves.push_back(new Move(tile, this, testing, testing->piece));
        }
    }
}

void King::calculateAvailableMoves(Tile*** tiles) {
    moves.clear();

    Tile* testing;

    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            if (tile->row + i >= 0 && tile->row + i <= 7 && tile->column + j >= 0 && tile->column + j <= 7) {
                testing = tiles[tile->row + i][tile->column + j];
                if (!testing->piece) {
                    moves.push_back(new Move(tile, this, testing));
                }
                else if (testing->piece->owner != owner) {
                    moves.push_back(new Move(tile, this, testing, testing->piece));
                }
            }
        }
    }

    if (times_moved == 0 && tiles[tile->row][7]->piece && tiles[tile->row][7]->piece->times_moved == 0) {
        if (!tiles[tile->row][tile->column + 1]->piece && !tiles[tile->row][tile->column + 2]->piece) {
            moves.push_back(new MoveCastle(tile, tiles[tile->row][7], this, tiles[tile->row][7]->piece, tiles[tile->row][tile->column + 2], tiles[tile->row][tile->column + 1]));
        }
    }

    if (times_moved == 0 && tiles[tile->row][0]->piece && tiles[tile->row][0]->piece->times_moved == 0) {
        if (!tiles[tile->row][tile->column - 1]->piece && !tiles[tile->row][tile->column - 2]->piece && !tiles[tile->row][tile->column - 3]->piece) {
            moves.push_back(new MoveCastle(tile, tiles[tile->row][0], this, tiles[tile->row][0]->piece, tiles[tile->row][tile->column - 2], tiles[tile->row][tile->column - 1]));
        }
    }
}