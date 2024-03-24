#ifndef BOARDSETTINGS_HPP
#define BOARDSETTINGS_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

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
const sf::Color NAME_TEXT_FILL(255, 255, 255);

const sf::Vector2f ENDSCREEN_RECT_SIZE(250, 300);
const sf::Color ENDSCREEN_BACKGROUND_DRAW(229, 143, 42);
const sf::Color ENDSCREEN_BACKGROUND_WIN(129, 182, 76);
const string ENDSCREEN_FONT = "helvetica";
const int ENDSCREEN_FONTSIZE_NAME = 60;
const int ENDSCREEN_NAME_MAX_WIDTH = 150;
const int ENDSCREEN_NAME_WIDTH_BUFFER = -4;
const int ENDSCREEN_NAME_HEIGHT = -100;
const sf::Color ENDSCREEN_NAME_FILL(255, 255, 255);
const int ENDSCREEN_FONTSIZE_BY = 20;
const int ENDSCREEN_BY_WIDTH_BUFFER = -4;
const int ENDSCREEN_BY_HEIGHT = 0;
const sf::Color ENDSCREEN_BY_FILL(255, 255, 255);
const int ENDSCREEN_FONTSIZE_RESULT = 60;
const int ENDSCREEN_RESULT_MAX_WIDTH = 200;
const int ENDSCREEN_RESULT_WIDTH_BUFFER = -4;
const int ENDSCREEN_RESULT_HEIGHT = 64;
const sf::Color ENDSCREEN_RESULT_FILL(255, 255, 255);

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

const int LOBBY_PLAYER_NAME_RECT_WIDTH = TILESIZE - 14;
const int LOBBY_PLAYER_NAME_RECT_HEIGHT = 25;
const int LOBBY_PLAYER_NAME_RECT_WIDTH_OFFSET = 7;
const int LOBBY_PLAYER_NAME_RECT_HEIGHT_OFFSET = TILESIZE - 35;
const sf::Color LOBBY_PLAYER_NAME_RECT_COLOR(240, 235, 223);
const string LOBBY_PLAYER_NAME_FONT = "helvetica";
const int LOBBY_PLAYER_NAME_FONTSIZE = 18;
const int LOBBY_PLAYER_NAME_TEXT_HEIGHT_OFFSET = -12;
const sf::Color LOBBY_PLAYER_NAME_COLOR(0, 0, 0);
const float LOBBY_ARROW_SCALE = 0.7f;

const string LOBBY_START_PLACEHOLDER = "choose opponent";
const sf::Vector2f LOBBY_START_SIZE(250, 150);
const sf::Color LOBBY_START_UNCLICKABLE_COLOR(98, 98, 102);
const sf::Color LOBBY_START_CLICKABLE_COLOR(15, 189, 43);
const sf::Color LOBBY_START_ACTIVE_COLOR(210, 32, 39);
const string LOBBY_START_FONT = "helvetica";
const int LOBBY_START_TEXT_FONTSIZE = 42;
const int LOBBY_START_TEXT_HEIGHT_OFFSET = -30;
const sf::Color LOBBY_START_TEXT_COLOR(0, 0, 0);
const int LOBBY_START_SUBTEXT_FONTSIZE = 24;
const int LOBBY_START_SUBTEXT_HEIGHT_OFFSET = 20;
const sf::Color LOBBY_START_SUBTEXT_COLOR(0, 0, 0);

const sf::Vector2f LOBBY_EDIT_SIZE(60, 60);
const sf::Vector2f LOBBY_EDIT_OFFSET(110, 350);
const sf::Color LOBBY_EDIT_DEACTIVE_COLOR(25, 167, 210);
const sf::Color LOBBY_EDIT_ACTIVE_COLOR(14, 255, 42);

const int LOBBY_INPUT_PADDING = 10;

const string LOBBY_SEARCH_PLACEHOLDER = "search";
const sf::Vector2f LOBBY_SEARCH_SIZE(200, 50);
const int LOBBY_SEARCH_HEIGHT_OFFSET = -350;
const sf::Color LOBBY_SEARCH_COLOR(243, 243, 243);
const string LOBBY_SEARCH_FONT = "helvetica";
const int LOBBY_SEARCH_FONTSIZE = 24;
const int LOBBY_SEARCH_TEXT_MAX_WIDTH = 180;
const float LOBBY_SEARCH_TEXT_HEIGHT_OFFSET = 1.15f;
const sf::Color LOBBY_SEARCH_TEXT_COLOR(0, 0, 0);

const sf::Vector2f LOBBY_NAME_SIZE(200, 60);
const int LOBBY_NAME_WIDTH_OFFSET = -30;
const int LOBBY_NAME_HEIGHT_OFFSET = 350;
const sf::Color LOBBY_NAME_COLOR(254, 6, 142);
const string LOBBY_NAME_FONT = "helvetica";
const int LOBBY_NAME_FONTSIZE = 42;
const int LOBBY_NAME_TEXT_MAX_WIDTH = 180;
const float LOBBY_NAME_TEXT_HEIGHT_OFFSET = 1.15f;
const sf::Color LOBBY_NAME_TEXT_COLOR(0, 0, 0);

const int MAX_NAME_LENGTH = 8;

const sf::Vector2f LOBBY_INVITE_SIZE(225, 175);
const sf::Vector2f LOBBY_INVITE_POSITION(0, 200);
const sf::Color LOBBY_INVITE_COLOR(56, 121, 175);
const string LOBBY_INVITE_FONT = "helvetica";
const int LOBBY_INVITE_TEXT_FONTSIZE = 42;
const sf::Vector2f LOBBY_INVITE_TEXT_OFFSET(-8, -55);
const int LOBBY_INVITE_TEXT_MAX_WIDTH = 200;
const sf::Color LOBBY_INVITE_TEXT_COLOR(0, 0, 0);
const int LOBBY_INVITE_SUBTEXT_FONTSIZE = 32;
const sf::Vector2f LOBBY_INVITE_SUBTEXT_OFFSET(-5, -10);
const int LOBBY_INVITE_SUBTEXT_MAX_WIDTH = 150;
const sf::Color LOBBY_INVITE_SUBTEXT_COLOR(0, 0, 0);

const sf::Vector2f LOBBY_INVITE_V_SIZE(75, 45);
const sf::Vector2f LOBBY_INVITE_V_POSITION(-50, 255);
const sf::Color LOBBY_INVITE_V_COLOR(13, 249, 25);
const sf::Vector2f LOBBY_INVITE_X_SIZE(75, 45);
const sf::Vector2f LOBBY_INVITE_X_POSITION(50, 255);
const sf::Color LOBBY_INVITE_X_COLOR(249, 13, 25);

#endif