#ifndef BOARDSETTINGS_HPP
#define BOARDSETTINGS_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <chrono>

using namespace std;
typedef std::basic_string<char> string;

const enum PieceType { pawn, rook, knight, bishop, queen, king };
const enum GameResult { checkmate, stalemate, resignation, agreed_draw, threefold, fifty_moves, dead_position, timeout, insufficient_material, error_move, error_timer, error_sync, disconnected };

const sf::Color BACKGROUND_COLOR(37, 30, 26);

const int BUTTON_DISTANCE = 50;
const sf::Vector2f BUTTON_SIZE(70, 55);
const sf::Color BUTTON_RESIGN_COLOR(202, 52, 49);
const sf::Color BUTTON_DRAW_COLOR(229, 143, 42);
const float BUTTON_SPRITE_SCALE = 0.8f;

const int TILESIZE = 120;
const int SIDEBAR = 300;
const int WINDOW_HEIGHT = TILESIZE * 8;
const int WINDOW_WIDTH = TILESIZE * 8 + SIDEBAR;

const int TIMER_DISTANCE = 400;
const int TIMER_RECT_WIDTH = 160;
const int TIMER_RECT_HEIGHT = 60;
const int TIMER_FONTSIZE = 40;
const int TIMER_TEXT_HEIGHT_BUFFER = -8;
const int TIMER_TEXT_WIDTH_BUFFER = -4;
const string TIMER_FONT = "dejavu";
const sf::Color TIMER_RECT_COLOR_WHITE(155, 149, 146);
const sf::Color TIMER_RECT_COLOR_BLACK(47, 40, 36);
const sf::Color TIMER_TEXT_FILL_WHITE(102, 97, 92);
const sf::Color TIMER_TEXT_FILL_BLACK(135, 129, 126);

const int NAME_FONTSIZE = 60;
const int NAME_MAX_WIDTH = 150;
const int NAME_DISTANCE = 300;
const int NAME_HEIGHT_BUFFER = -20;
const int NAME_WIDTH_BUFFER = -4;
const string NAME_FONT = "helvetica";
const sf::Color NAME_TEXT_FILL = sf::Color::White;

const sf::Vector2f ENDSCREEN_RECT_SIZE(250, 300);
const sf::Color ENDSCREEN_BACKGROUND_DRAW(229, 143, 42);
const sf::Color ENDSCREEN_BACKGROUND_WIN(129, 182, 76);
const string ENDSCREEN_FONT = "helvetica";
const int ENDSCREEN_FONTSIZE_NAME = 60;
const int ENDSCREEN_NAME_MAX_WIDTH = 150;
const int ENDSCREEN_NAME_WIDTH_BUFFER = -4;
const int ENDSCREEN_NAME_HEIGHT = -100;
const sf::Color ENDSCREEN_NAME_FILL = sf::Color::White;
const int ENDSCREEN_FONTSIZE_BY = 20;
const int ENDSCREEN_BY_WIDTH_BUFFER = -4;
const int ENDSCREEN_BY_HEIGHT = 0;
const sf::Color ENDSCREEN_BY_FILL = sf::Color::White;
const int ENDSCREEN_FONTSIZE_RESULT = 60;
const int ENDSCREEN_RESULT_MAX_WIDTH = 200;
const int ENDSCREEN_RESULT_WIDTH_BUFFER = -4;
const int ENDSCREEN_RESULT_HEIGHT = 64;
const sf::Color ENDSCREEN_RESULT_FILL = sf::Color::White;

const sf::Color TILE_BLACK(188, 161, 125);
const sf::Color TILE_WHITE(74, 51, 28);
const sf::Color TILE_BLACK_SELECT(91, 159, 189);
const sf::Color TILE_WHITE_SELECT(104, 174, 202);
const sf::Color TILE_BLACK_MOVEABLE(210, 94, 75);
const sf::Color TILE_WHITE_MOVEABLE(225, 110, 88);
const sf::Color TILE_BLACK_RECENT(161, 181, 82);
const sf::Color TILE_WHITE_RECENT(176, 198, 97);
const sf::Color TILE_BLACK_CHECK(230, 155, 13);
const sf::Color TILE_WHITE_CHECK(241, 167, 24);
const sf::Color TILE_CHECKMATE(242, 41, 22);

const int SYNC_MAX_DIFFERENCE = 10;

#endif