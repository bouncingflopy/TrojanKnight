#include "pieces.h"
#include "tiles.h"
#include "boardsettings.h"
#include "sidebar.h"
#include "player.h"
#include "moves.h"
#include "board.h"

Board::Board(int m) : me(m) {
    players = new Player * [2];
    players[0] = new Player(0, "black", me);
    players[1] = new Player(1, "white", me);

    tiles = createTiles();
    pieces = createPieces(tiles, players);

    start_time = chrono::high_resolution_clock::now();

    timers = new Timer*[2];
    timers[0] = new Timer(this, 80, 0);
    timers[1] = new Timer(this, 80, 1);

    timers[1]->start();

    buttons.push_back(new ButtonDraw(this));
    buttons.push_back(new ButtonResign(this));

    turn = 1;

    calculateAllAvailableMoves();

    boardHistory.push_back(boardFEN());
}

void Board::updateClocks() { // change to complete update (timers, input)
    game_tick = chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - start_time).count();
    for (int i = 0; i < 2; i++) {
        timers[i]->update();
    }
}

void Board::draw(sf::RenderWindow& window) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            window.draw(tiles[i][j]->rect);
        }
    }

    for (int i = 0; i < pieces.size(); i++) {
        window.draw(pieces[i]->sprite);
    }

    if (waiting_promotion) {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                window.draw(promotion_tiles[i][j]->rect);
                window.draw(promotion_tiles[i][j]->piece->sprite);
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        window.draw(timers[i]->rect);
        window.draw(timers[i]->text);
    }

    for (int i = 0; i < buttons.size(); i++) {
        window.draw(buttons[i]->rect);
        window.draw(buttons[i]->sprite);
    }

    for (int i = 0; i < 2; i++) {
        window.draw(players[i]->name_object->text);
    }

    if (!enabled) {
        window.draw(end_screen->rect);
        for (int i = 0; i < 3; i++) {
            window.draw(end_screen->text[i]);
        }
    }
}

Tile*** Board::createTiles() {
    Tile*** tiles = new Tile * *[8];

    for (int i = 0; i < 8; i++) {
        tiles[i] = new Tile * [8];
        for (int j = 0; j < 8; j++) {
            tiles[i][j] = new Tile(i, j, me);
        }
    }

    return tiles;
}

vector<Piece*> Board::createPieces(Tile*** tiles, Player** players) {
    vector<Piece*> pieces;

    pieces.push_back(new Rook(tiles[0][0], players[0]));
    pieces.push_back(new Knight(tiles[0][1], players[0]));
    pieces.push_back(new Bishop(tiles[0][2], players[0]));
    pieces.push_back(new Queen(tiles[0][3], players[0]));
    pieces.push_back(new King(tiles[0][4], players[0]));
    pieces.push_back(new Bishop(tiles[0][5], players[0]));
    pieces.push_back(new Knight(tiles[0][6], players[0]));
    pieces.push_back(new Rook(tiles[0][7], players[0]));

    for (int i = 0; i < 8; i++) {
        pieces.push_back(new Pawn(tiles[1][i], players[0]));
    }

    for (int i = 0; i < 8; i++) {
        pieces.push_back(new Pawn(tiles[6][i], players[1]));
    }

    pieces.push_back(new Rook(tiles[7][0], players[1]));
    pieces.push_back(new Knight(tiles[7][1], players[1]));
    pieces.push_back(new Bishop(tiles[7][2], players[1]));
    pieces.push_back(new Queen(tiles[7][3], players[1]));
    pieces.push_back(new King(tiles[7][4], players[1]));
    pieces.push_back(new Bishop(tiles[7][5], players[1]));
    pieces.push_back(new Knight(tiles[7][6], players[1]));
    pieces.push_back(new Rook(tiles[7][7], players[1]));

    return pieces;
}

Tile* Board::getTileAtPosition(int x, int y, bool flipped) {
    if (waiting_promotion) {
        if (promotion_destination->column * TILESIZE <= x && (promotion_destination->column + 1) * TILESIZE >= x) {
            if (flipped) {
                if ((7 - promotion_destination->row) * TILESIZE <= y && (7 - (promotion_destination->row - 1)) * TILESIZE >= y) {
                    return promotion_tiles[(y - ((7 - promotion_destination->row) * TILESIZE)) / (TILESIZE / 2)][(x - (promotion_destination->column * TILESIZE)) / (TILESIZE / 2)];
                }
            }
            else {
                if (promotion_destination->row * TILESIZE <= y && (promotion_destination->row + 1) * TILESIZE >= y) {
                    return promotion_tiles[(y - (promotion_destination->row * TILESIZE)) / (TILESIZE / 2)][(x - (promotion_destination->column * TILESIZE)) / (TILESIZE / 2)];
                }
            }
        }
    }

    return tiles[flipped ? 7 - (y / TILESIZE) : y / TILESIZE][x / TILESIZE];
}

bool Board::noMoves(Player* player) {
    for (int i = 0; i < pieces.size(); i++) {
        if (pieces[i]->owner == player) {
            if (!pieces[i]->moves.empty()) {
                return false;
            }
        }
    }

    return true;
}

bool Board::checkCheck(Player* player) {
    for (int i = 0; i < pieces.size(); i++) {
        if (pieces[i]->owner != player) {
            for (int j = 0; j < pieces[i]->moves.size(); j++) {
                if (pieces[i]->moves[j]->eat && pieces[i]->moves[j]->eat == player->king_piece) {
                    return true;
                }
            }
        }
    }

    return false;
}

void Board::makeDubiousMove(Move* move) {
    if (move->eat) {
        move->eat->tile->piece = NULL;
        killPiece(move->eat);
    }

    move->piece->moveTo(move->destination, 1);
    if (move->castle) {
        ((MoveCastle*)move)->piece_rook->moveTo(((MoveCastle*)move)->destination_rook, 1);
    }

    if (move->promotion) {
        killPiece(move->piece);
        move->destination->piece = new Queen(move->destination, players[turn]);
        pieces.push_back(move->destination->piece);
    }

    moveHistory.push_back(move);
    turn = 1 - turn;

    for (int i = 0; i < pieces.size(); i++) {
        pieces[i]->calculateAvailableMoves(tiles, move);
    }
}

void Board::undoDubiousMove() {
    Move* move = moveHistory.back();

    if (move->promotion) {
        revivePiece(move->piece);
        killPiece(move->destination->piece);
        move->destination->piece = move->piece;
    }

    move->piece->moveTo(move->source, -1);
    if (move->castle) {
        ((MoveCastle*)move)->piece_rook->moveTo(((MoveCastle*)move)->source_rook, -1);
    }

    if (move->eat) {
        revivePiece(move->eat);
        if (move->eat_tile) {
            move->eat_tile->piece = move->eat;
        }
        else {
            move->destination->piece = move->eat;
        }
    }

    moveHistory.pop_back();
    turn = 1 - turn;
}

vector<Move*>** Board::createMoveCache() {
    vector<Move*>** move_cache = new vector<Move*>*[8];

    for (int i = 0; i < 8; i++) {
        move_cache[i] = new vector<Move*>[8];
        for (int j = 0; j < 8; j++) {
            if (tiles[i][j]->piece) {
                move_cache[i][j] = tiles[i][j]->piece->moves;
            }
        }
    }

    return move_cache;
}

void Board::loadMoveCache(vector<Move*>** move_cache) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (tiles[i][j]->piece) {
                tiles[i][j]->piece->moves = move_cache[i][j];
            }
        }
    }
}

void Board::calculateAllAvailableMoves() {
    for (int i = 0; i < pieces.size(); i++) {
        pieces[i]->calculateAvailableMoves(tiles, (moveHistory.size() > 0) ? moveHistory.back() : NULL);
    }

    bool leads_to_check;
    for (int move = 0; move < players[turn]->king_piece->moves.size(); move++) {
        if (players[turn]->king_piece->moves[move]->castle) {
            if (checkCheck(players[turn])) {
                players[turn]->king_piece->moves.erase(players[turn]->king_piece->moves.begin() + move);
            }
            else {
                move_cache = createMoveCache();
                makeDubiousMove(players[turn]->king_piece->moves[move]);

                leads_to_check = checkCheck(players[1 - turn]);

                undoDubiousMove();
                loadMoveCache(move_cache);

                if (leads_to_check) {
                    players[turn]->king_piece->moves.erase(players[turn]->king_piece->moves.begin() + move);
                }
                else {
                    makeDubiousMove(new Move(players[turn]->king_piece->moves[move]->source, players[turn]->king_piece, ((MoveCastle*)players[turn]->king_piece->moves[move])->destination_rook));

                    leads_to_check = checkCheck(players[1 - turn]);

                    undoDubiousMove();
                    loadMoveCache(move_cache);

                    if (leads_to_check) {
                        players[turn]->king_piece->moves.erase(players[turn]->king_piece->moves.begin() + move);
                    }
                }
            }
        }
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (tiles[i][j]->piece && tiles[i][j]->piece->owner == players[turn]) {
                for (int move = 0; move < tiles[i][j]->piece->moves.size(); move++) {
                    move_cache = createMoveCache();
                    makeDubiousMove(tiles[i][j]->piece->moves[move]);

                    leads_to_check = checkCheck(players[1 - turn]);

                    undoDubiousMove();
                    loadMoveCache(move_cache);

                    if (leads_to_check) {
                        tiles[i][j]->piece->moves.erase(tiles[i][j]->piece->moves.begin() + move);
                        move--;
                    }
                }
            }
        }
    }
}

void Board::endGame(GameResult result) {
    endGame(result, -1);
}

void Board::endGame(GameResult result, int player) {
    game_result = result;

    if (result == timeout) {
        vector<Piece*> my_pieces;

        for (int i = 0; i < pieces.size(); i++) {
            if (pieces[i]->owner == players[1 - turn]) my_pieces.push_back(pieces[i]);
        }

        if (my_pieces.size() == 1) game_result = insufficient_material;

        if (my_pieces.size() == 2 && (my_pieces[0]->type == knight || my_pieces[1]->type == knight)) game_result = insufficient_material;

        bool bishop_check = true;
        int bishop_color = NULL;
        for (int i = 0; i < my_pieces.size(); i++) {
            if (my_pieces[i]->type == bishop) {
                if (bishop_color && my_pieces[i]->tile->base_color != bishop_color) {
                    bishop_check = false;
                }
                else {
                    bishop_color = my_pieces[i]->tile->base_color;
                }
            }
            else if (my_pieces[i]->type != king) {
                bishop_check = false;
            }
        }
        if (bishop_check) {
            game_result = insufficient_material;
        }
    }

    if (game_result == timeout || game_result == resignation || game_result == disconnected) winner = players[1 - player]->name;
    exitGame();
}

bool Board::checkEndGame() {
    if (noMoves(players[turn])) {
        if (checkCheck(players[turn])) {
            players[turn]->king_piece->tile->highlightCheckmate();
            game_result = checkmate;
            winner = players[1 - turn]->name;
            return true;
        }
        else {
            game_result = stalemate;
            return true;
        }
    }

    int count = 0;
    for (int i = 0; i < boardHistory.size(); i++) {
        if (boardHistory.back() == boardHistory[i]) count++;
    }
    if (count == 3) {
        game_result = threefold;
        return true;
    }

    if (dullMoves == 100) {
        game_result = fifty_moves;
        return true;
    }

    if (pieces.size() == 2) {
        game_result = dead_position;
        return true;
    }
    if (pieces.size() == 3) {
        if (pieces[0]->type == knight || pieces[1]->type == knight || pieces[2]->type == knight) {
            game_result = dead_position;
            return true;
        }
    }
    bool bishop_check = true;
    int bishop_color = NULL;
    for (int i = 0; i < pieces.size(); i++) {
        if (pieces[i]->type == bishop) {
            if (bishop_color && pieces[i]->tile->base_color != bishop_color) {
                bishop_check = false;
            }
            else {
                bishop_color = pieces[i]->tile->base_color;
            }
        }
        else if (pieces[i]->type != king) {
            bishop_check = false;
        }
    }
    if (bishop_check) {
        game_result = dead_position;
        return true;
    }

    return false;
}

void Board::exitGame() {
    enabled = false;
    timers[turn]->stop();

    end_screen = new EndScreen(game_result, winner);
}

void Board::changeTurn() {
    turn = 1 - turn;

    timers[1 - turn]->stop();
    timers[turn]->start();

    addLogs();
    calculateAllAvailableMoves();

    if (players[1 - turn]->king_piece->tile->checked) {
        players[1 - turn]->king_piece->tile->unhighlightCheck();
    }
    else if (moveHistory.back()->piece->type == king && moveHistory.back()->source->checked) {
        moveHistory.back()->source->unhighlightCheck();
    }

    if (checkEndGame()) {
        exitGame();
    }

    else if (checkCheck(players[turn])) {
        players[turn]->king_piece->tile->highlightCheck();
    }
}

void Board::killPiece(Piece* piece) {
    for (int i = 0; i < pieces.size(); i++) {
        if (pieces[i] == piece) {
            pieces.erase(pieces.begin() + i);
        }
    }

    deadPieces.push_back(piece);
}

void Board::revivePiece(Piece* piece) {
    for (int i = 0; i < deadPieces.size(); i++) {
        if (deadPieces[i] == piece) {
            deadPieces.erase(deadPieces.begin() + i);
        }
    }

    pieces.push_back(piece);
}

string Board::boardFEN() {
    string fen;
    string row;
    int empty;

    for (int i = 0; i < 8; i++) {
        row;
        empty = 0;

        for (int j = 0; j < 8; j++) {
            if (tiles[i][j]->piece) {
                if (empty > 0) {
                    row += to_string(empty);
                    empty = 0;
                }

                if (tiles[i][j]->piece->type == pawn) row += (tiles[i][j]->piece->owner == players[1]) ? "P" : "p";
                if (tiles[i][j]->piece->type == knight) row += (tiles[i][j]->piece->owner == players[1]) ? "N" : "n";
                if (tiles[i][j]->piece->type == bishop) row += (tiles[i][j]->piece->owner == players[1]) ? "B" : "b";
                if (tiles[i][j]->piece->type == rook) row += (tiles[i][j]->piece->owner == players[1]) ? "R" : "r";
                if (tiles[i][j]->piece->type == queen) row += (tiles[i][j]->piece->owner == players[1]) ? "Q" : "q";
                if (tiles[i][j]->piece->type == king) row += (tiles[i][j]->piece->owner == players[1]) ? "K" : "k";
            }
            else {
                empty++;
            }
        }

        if (empty > 0) {
            row += to_string(empty);
        }

        fen += row + ((i < 7) ? "/" : " ");
    }

    fen += ((turn == 1) ? "w" : "b") + string(" ");

    string castle;
    if (players[1]->king_piece->times_moved == 0 && tiles[7][7]->piece && tiles[7][7]->piece->times_moved == 0) castle += "K";
    if (players[1]->king_piece->times_moved == 0 && tiles[7][0]->piece && tiles[7][0]->piece->times_moved == 0) castle += "Q";
    if (players[0]->king_piece->times_moved == 0 && tiles[0][7]->piece && tiles[0][7]->piece->times_moved == 0) castle += "k";
    if (players[0]->king_piece->times_moved == 0 && tiles[0][0]->piece && tiles[0][0]->piece->times_moved == 0) castle += "q";

    fen += ((castle.empty()) ? "-" : castle) + " ";

    string en_passant = "-";
    if (moveHistory.size() > 0) {
        if (moveHistory.back()->piece->type == pawn && abs(moveHistory.back()->source->row - moveHistory.back()->destination->row) == 2) {
            Tile* d = moveHistory.back()->destination;

            if (d->column > 0 && tiles[d->row][d->column - 1]->piece && tiles[d->row][d->column - 1]->piece->type == pawn && tiles[d->row][d->column - 1]->piece->owner != moveHistory.back()->piece->owner) {
                en_passant = to_string((d->row == 4) ? 3 : 6) + to_string(d->column);
            }
            else if (d->column < 7 && tiles[d->row][d->column + 1]->piece && tiles[d->row][d->column + 1]->piece->type == pawn && tiles[d->row][d->column + 1]->piece->owner != moveHistory.back()->piece->owner) {
                en_passant = to_string((d->row == 4) ? 3 : 6) + to_string(d->column);
            }
        }
    }
    fen += en_passant;

    return fen;
}

void Board::addLogs() {
    if (moveHistory.back()->piece->type == pawn || moveHistory.back()->eat) dullMoves = 0;
    else dullMoves++;

    boardHistory.push_back(boardFEN());
}

void Board::makeMove(Move* move) {
    if (move->eat) {
        move->eat->tile->piece = NULL;
        killPiece(move->eat);
    }

    move->piece->moveTo(move->destination, 1);
    if (move->castle) {
        ((MoveCastle*)move)->piece_rook->moveTo(((MoveCastle*)move)->destination_rook, 1);
    }

    if (move->promotion) {
        killPiece(move->piece);
        if (((MovePromotion*)move)->piece_type == knight) move->destination->piece = new Knight(move->destination, players[turn]);
        if (((MovePromotion*)move)->piece_type == bishop) move->destination->piece = new Bishop(move->destination, players[turn]);
        if (((MovePromotion*)move)->piece_type == rook) move->destination->piece = new Rook(move->destination, players[turn]);
        if (((MovePromotion*)move)->piece_type == queen) move->destination->piece = new Queen(move->destination, players[turn]);
        pieces.push_back(move->destination->piece);
    }

    if (moveHistory.size() > 0) {
        moveHistory[moveHistory.size() - 1]->unhighlight();
    }
    moveHistory.push_back(move);
    move->highlight();

    appendCPNP(moveToCPNP(move));

    changeTurn();
}

/*
void Board::undoMove() {
    Move* move = moveHistory.back();

    if (move->promotion) {
        revivePiece(move->piece);
        killPiece(move->destination->piece);
        move->destination->piece = move->piece;
    }

    move->piece->moveTo(move->source, -1);
    if (move->castle) {
        ((MoveCastle*)move)->piece_rook->moveTo(((MoveCastle*)move)->source_rook, -1);
    }

    if (move->eat) {
        revivePiece(move->eat);
        if (move->eat_tile) {
            move->eat_tile->piece = move->eat;
        }
        else {
            move->destination->piece = move->eat;
        }
    }

    move->unhighlight();
    moveHistory.pop_back();
    if (moveHistory.size() > 0) {
        moveHistory.back()->highlight();
    }

    changeTurn();
}
*/

void Board::selectTile(Tile* tile) {
    tile->select();
    tile->piece->highlightMoves();
    selected_tile = tile;
}

void Board::deselectTile(Tile* tile) {
    tile->deselect();
    tile->piece->unhighlightMoves();
    selected_tile = NULL;
}

void Board::displayPromotion(Move* move) {
    killPiece(move->piece);

    waiting_promotion = true;
    promotion_destination = move->destination;
    promotion_piece = move->piece;
    promotion_move = move;

    promotion_tiles = new PromotionTile * *[2];
    for (int i = 0; i < 2; i++) {
        promotion_tiles[i] = new PromotionTile * [2];
        for (int j = 0; j < 2; j++) {
            promotion_tiles[i][j] = new PromotionTile(i, j, promotion_destination);
        }
    }

    new PromotionKnight(promotion_tiles[0][0], players[turn]);
    new PromotionBishop(promotion_tiles[0][1], players[turn]);
    new PromotionRook(promotion_tiles[1][0], players[turn]);
    new PromotionQueen(promotion_tiles[1][1], players[turn]);
}

void Board::handlePress(int x, int y) {
    if (x <= TILESIZE * 8 && y <= TILESIZE * 8) {
        if (turn == turn) { // turn == me
            Tile* pressed = getTileAtPosition(x, y, me == 0);

            if (waiting_promotion) {
                revivePiece(promotion_move->piece);
                waiting_promotion = false;

                if (pressed->promotion_tile) {
                    makeMove(((Pawn*)promotion_piece)->getMove(promotion_destination, ((PromotionTile*)pressed)->piece_type));
                }
            }
            else if (!selected_tile) {
                if (pressed->piece && pressed->piece->owner == players[turn]) {
                    selectTile(pressed);
                }
            }
            else {
                if (pressed->selected) {
                    deselectTile(selected_tile);
                }
                else if (pressed->piece && pressed->piece->owner == players[turn]) {
                    deselectTile(selected_tile);
                    selectTile(pressed);
                }
                else if (selected_tile->piece->getMove(pressed)) {
                    Move* move = selected_tile->piece->getMove(pressed);
                    deselectTile(selected_tile);

                    if (!move->promotion) {
                        makeMove(move);
                    }
                    else {
                        displayPromotion(move);
                    }
                }
                else {
                    deselectTile(selected_tile);
                }
            }
        }
    }
    else {
        for (int i = 0; i < buttons.size(); i++) {
            if (buttons[i]->checkClick(x, y)) buttons[i]->click();
        }
    }
}