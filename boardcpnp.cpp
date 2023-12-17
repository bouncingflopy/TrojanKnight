#include <iostream>

#include "board.h"
#include "tiles.h"
#include "pieces.h"
#include "moves.h"
#include "player.h"
#include "sidebar.h"

void Board::appendCPNP(string data) {
	if (!message_waiting) message_waiting = true;
	message += "\n" + data;
}

string Board::moveToCPNP(Move* move) {
	string line = "move";
	line += " " + getNotationFromTile(move->source);
	line += " " + getNotationFromTile(move->destination);
	if (move->promotion) {
		if (((MovePromotion*)move)->piece_type == knight) line += " knight";
		else if (((MovePromotion*)move)->piece_type == bishop) line += " bishop";
		else if (((MovePromotion*)move)->piece_type == rook) line += " rook";
		else if (((MovePromotion*)move)->piece_type == queen) line += " queen";
	}

	return line;
}

string Board::getNotationFromTile(Tile* tile) {
	char letter = 'a' + tile->column;
	char number = '1' + (7 - tile->row);
	string notation = "";
	notation += letter;
	notation += number;

	return notation;
}

Tile* Board::getTileFromNotation(string notation) {
	int letter = notation[0] - 'a';
	int number = notation[1] - '1';

	if (letter >= 0 && letter <= 7 && number >= 0 && number <= 7) return tiles[7 - number][letter];
	else return NULL;
}

void Board::decodeCPNP(string message) {
	vector<string> lines;
	istringstream stream(message);
	string temp;
	while (getline(stream, temp, '\n')) lines.push_back(temp);

	for (string line : lines) {
		vector<string> words;
		istringstream stream(line);
		string temp;
		while (getline(stream, temp, ' ')) words.push_back(temp);

		if (words[0] == "move") {
			Move* move = NULL;

			if (getTileFromNotation(words[1]) && getTileFromNotation(words[2])) {
				if (getTileFromNotation(words[1])->piece && getTileFromNotation(words[1])->piece->owner->color == 1 - me) {
					if (words.size() == 3) {
						if (getTileFromNotation(words[1])->piece) move = getTileFromNotation(words[1])->piece->getMove(getTileFromNotation(words[2]));
					}
					else if (words.size() == 4) {
						if (getTileFromNotation(words[1])->piece->type == pawn) {
							if (getTileFromNotation(words[1])->piece->getMove(getTileFromNotation(words[2]))->promotion) {
								if (words[3] == "knight") move = ((Pawn*)(getTileFromNotation(words[1])->piece))->getMove(getTileFromNotation(words[2]), knight);
								else if (words[3] == "bishop") move = ((Pawn*)(getTileFromNotation(words[1])->piece))->getMove(getTileFromNotation(words[2]), bishop);
								else if (words[3] == "rook") move = ((Pawn*)(getTileFromNotation(words[1])->piece))->getMove(getTileFromNotation(words[2]), rook);
								else if (words[3] == "queen") move = ((Pawn*)(getTileFromNotation(words[1])->piece))->getMove(getTileFromNotation(words[2]), queen);
							}
						}
					}
				}
			}
			
			if (move) {
				makeMove(move);
			}
			else {
				appendCPNP("error move");
				endGame(error_move);
			}
		}
		else if (words[0] == "resign") {
			endGame(resignation, 1 - me);
		}
		else if (words[0] == "draw") {
			if (words[1] == "offer") {
				((ButtonDraw*)buttons[0])->updateState(true, false, false);
			}
			else if (words[1] == "accept") {
				((ButtonDraw*)buttons[0])->updateState(false, false, true);
			}
			else if (words[1] == "cancel") {
				((ButtonDraw*)buttons[0])->updateState(false, true, false);
			}
		}
		else if (words[0] == "timer") {
			if (words.size() == 4) {
				if (abs(stoi(words[3]) - game_tick) >= SYNC_MAX_DIFFERENCE) {
					appendCPNP("error sync");
					endGame(error_sync);
				}
				else if (stoi(words[3]) >= last_reported_game_tick) {
					last_reported_game_tick = stoi(words[3]);

					if (words[1] == "start") {
						timers[1 - me]->reported_start_time = stoi(words[2]);
						timers[1 - me]->reported_start_game_tick = stoi(words[3]);
					}
					else if (words[1] == "stop") {
						if (abs((stoi(words[2]) - timers[1 - me]->reported_start_time) - (stoi(words[3]) - timers[1 - me]->reported_start_game_tick)) <= 1) {
							timers[1 - me]->stop();
							timers[1 - me]->time = stoi(words[2]);
						}
						else {
							appendCPNP("error timer");
							endGame(error_timer);
						}
					}
					else if (words[1] == "dynamic") {
						if (abs((stoi(words[2]) - timers[1 - me]->reported_start_time) - (stoi(words[3]) - timers[1 - me]->reported_start_game_tick)) <= 1) {
							timers[1 - me]->time = stoi(words[2]) + game_tick - stoi(words[3]);
						}
						else {
							appendCPNP("error timer");
							endGame(error_timer);
						}
					}
				}
				else {
					appendCPNP("error timer");
					endGame(error_timer);
				}
			}
			else if (words[2] == "timeout") {
				endGame(timeout, 1 - me);
			}
		}
		else if (words[0] == "error") {
			if (words[1] == "move") endGame(error_move);
			else if (words[1] == "timer") endGame(error_timer);
			else if (words[1] == "sync") endGame(error_sync);
		}
	}
}

/*

pnp
	chess offer white rapid
	chess accept
	chess reject
	cpnp
		timer dynamic 147 837
		timer start 132 837
		timer stop 104 837
		timer timeout
		move e7 e8 queen
		draw accept
		draw offer
		draw cancel
		resign
		error move
		error timer
		error sync

*/

string Board::popMessage() {
	message_waiting = false;

	string temp = message;
	message = "";

	return temp;
}