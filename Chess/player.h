#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>

using namespace std;

class Piece;
class Name;

class Player {
public:
    int color;
    Piece* king_piece;
    string name;
    Name* name_object;

    Player(int c, string n, int m);
    void setKing(Piece* k);
};

#endif