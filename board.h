#ifndef BOARD_HPP
#define BOARD_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>

#include "boardsettings.h"
#include "connection.h"

using namespace std;

class Tile;
class Piece;
class Move;
class Player;
enum GameResult;
class PromotionTile;
class EndScreen;
class Timer;
class Button;
class Connection;

class Board {
public:
    Tile*** tiles;
    vector<Piece*> pieces;
    bool running = true;
    vector<Move*> moveHistory;
    Player** players;
    int turn;
    int me;
    Tile* selected_tile;
    vector<Piece*> deadPieces;
    vector<Move*>** move_cache;
    bool enabled = true;
    GameResult game_result;
    string winner;
    bool waiting_promotion = false;
    Tile* promotion_destination;
    PromotionTile*** promotion_tiles;
    Piece* promotion_piece;
    Move* promotion_move;
    int dullMoves = 0;
    vector<string> boardHistory;
    Timer** timers;
    vector<Button*> buttons;
    EndScreen* end_screen;
    bool message_waiting = false;
    string message;
    chrono::time_point<chrono::high_resolution_clock> start_time;
    int game_tick = 0;
    int last_reported_game_tick = 0;
    shared_ptr<Connection> connection;

    Board(shared_ptr<Connection> c, int m);
    void updateClocks();
    void draw(sf::RenderWindow& window);
    Tile*** createTiles();
    vector<Piece*> createPieces(Tile*** tiles, Player** players);
    Tile* getTileAtPosition(int x, int y, bool flipped);
    bool noMoves(Player* player);
    bool checkCheck(Player* player);
    void makeDubiousMove(Move* move);
    void undoDubiousMove();
    vector<Move*>** createMoveCache();
    void loadMoveCache(vector<Move*>** move_cache);
    void calculateAllAvailableMoves();
    void endGame(GameResult result);
    void endGame(GameResult result, int player);
    bool checkEndGame();
    void exitGame();
    void changeTurn();
    void killPiece(Piece* piece);
    void revivePiece(Piece* piece);
    string boardFEN();
    void addLogs();
    void makeMove(Move* move);
    //void undoMove();
    void selectTile(Tile* tile);
    void deselectTile(Tile* tile);
    void displayPromotion(Move* move);
    void handlePress(int x, int y);
    void appendCPNP(string data);
    string moveToCPNP(Move* move);
    string getNotationFromTile(Tile* tile);
    Tile* getTileFromNotation(string notation);
    void decodeCPNP(string message);
    string popMessage();
};

#endif