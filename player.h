#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>

typedef std::basic_string<char> string;

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