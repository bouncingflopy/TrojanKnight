#ifndef SIDEBAR_HPP
#define SIDEBAR_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <chrono>

using namespace std;

class Board;
enum GameResult;

class Name {
public:
    string name;
    sf::Font font;
    sf::Text text;
    int player;
    int me;

    Name();
    Name(string n, int p, int m);
    void createSprite();
};

class Timer {
public:
    double time = 0;
    chrono::time_point<chrono::high_resolution_clock> start_time;
    bool running = false;
    bool stopped = false;
    int initial_time;
    int seconds;
    int display_seconds;
    int display_minutes;
    int player;
    sf::RectangleShape rect;
    sf::Font font;
    sf::Text text;
    Board* board;
    int me;
    int reported_start_game_tick;
    int reported_start_time;
    int last_sent_time = 0;

    Timer();
    Timer(Board* b, int s, int p);
    void createSprite();
    void start();
    void stop();
    void update();
    void updateText();
};

class Button {
public:
    Board* board;
    sf::RectangleShape rect;
    sf::Texture texture;
    sf::Sprite sprite;

    Button();
    Button(Board* b);
    void createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color);
    void createSprite(string file);
    bool checkClick(int x, int y);
    virtual void click();
};

class ButtonResign : public Button {
public:
    ButtonResign();
    ButtonResign(Board* b);
    void click();
};

class ButtonDraw : public Button {
public:
    bool offer_sent = false;
    bool offer_received = false;

    ButtonDraw();
    ButtonDraw(Board* b);
    void updateState(bool received, bool canceled, bool accepted);
    void click();
};

class EndScreen {
public:
    Board* board;
    GameResult result;
    string winner;
    bool tie;
    sf::RectangleShape rect;
    sf::Font font;
    sf::Text text[3];

    EndScreen(Board* b, GameResult r, string w);
    string stringify(GameResult result);
    bool checkClick(int x, int y);
    void click();
};

#endif