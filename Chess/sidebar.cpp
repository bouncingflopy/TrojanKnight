#include "pieces.h"
#include "tiles.h"
#include "boardsettings.h"
#include "sidebar.h"
#include "player.h"
#include "moves.h"
#include "board.h"
#include "resourceloader.h"

Name::Name(string n, int p, int m) : name(n), player(p), me(m) {
    createSprite();
};

void Name::createSprite() {
    font = *resource::loadFont(NAME_FONT);
    text.setFont(font);
    text.setString(name);
    text.setStyle(sf::Text::Bold);

    text.setCharacterSize(NAME_FONTSIZE);
    int scaled_font_size = NAME_FONTSIZE;
    if (text.getGlobalBounds().width > NAME_MAX_WIDTH) {
        float scale = 1.f * text.getGlobalBounds().width / NAME_MAX_WIDTH;
        scaled_font_size = static_cast<int>(NAME_FONTSIZE / scale);
        text.setCharacterSize(scaled_font_size);
    }

    text.setPosition(sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - text.getGlobalBounds().width / 2 + NAME_WIDTH_BUFFER, TILESIZE * 4 + (((player == me) ? 1 : -1) * (NAME_DISTANCE)) - text.getGlobalBounds().height / 2 + NAME_HEIGHT_BUFFER));
    text.setFillColor(NAME_TEXT_FILL);
}

Timer::Timer(Board* b, int s, int p) : board(b), initial_time(s), seconds(s), player(p) {
    me = board->me;
    createSprite();
};

void Timer::createSprite() {
    rect.setSize(sf::Vector2f(TIMER_RECT_WIDTH, TIMER_RECT_HEIGHT));
    rect.setPosition(sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - rect.getSize().x / 2, TILESIZE * 4 + (((player == me) ? 1 : -1) * (TIMER_DISTANCE)) - rect.getSize().y / 2));
    rect.setFillColor((player == 1) ? TIMER_RECT_COLOR_WHITE : TIMER_RECT_COLOR_BLACK);

    font = *resource::loadFont(TIMER_FONT);

    text.setFont(font);
    text.setCharacterSize(TIMER_FONTSIZE);
    text.setString("00:00");
    text.setPosition(sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - text.getGlobalBounds().width / 2 + TIMER_TEXT_WIDTH_BUFFER, TILESIZE * 4 + (((player == me) ? 1 : -1) * (TIMER_DISTANCE)) - text.getGlobalBounds().height / 2 + TIMER_TEXT_HEIGHT_BUFFER));
    text.setFillColor((player == 1) ? TIMER_TEXT_FILL_WHITE : TIMER_TEXT_FILL_BLACK);
    text.setStyle(sf::Text::Bold);

    updateText();
}

void Timer::start() {
    start_time = chrono::high_resolution_clock::now();
    running = true;
    stopped = false;
    if (player == me) board->appendCPNP("timer start " + to_string((int)time) + " " + to_string(board->game_tick));
}

void Timer::stop() {
    if (stopped) return;
    stopped = true;

    update();
    running = false;
    if (player == me) board->appendCPNP("timer stop " + to_string((int)time) + " " + to_string(board->game_tick));
}

void Timer::update() {
    if (!running) return;

    time += chrono::duration<double>(chrono::high_resolution_clock::now() - start_time).count();
    start_time = chrono::high_resolution_clock::now();

    seconds = initial_time - round(time);
    updateText();

    if (seconds == 0) {
        if (player == me) {
            board->endGame(timeout, player);
            board->appendCPNP("timer timeout");
        }
        else {
            running = false;
        }
    }

    if (player == me && time - last_sent_time >= 3) {
        board->appendCPNP("timer dynamic " + to_string((int)time) + " " + to_string(board->game_tick));
        last_sent_time = time;
    }
}

void Timer::updateText() {
    display_minutes = seconds / 60;
    display_seconds = seconds - (60 * display_minutes);
    text.setString(((display_minutes >= 10) ? "" : "0") + ((display_minutes > 0) ? to_string(display_minutes) : "0") + ":" + ((display_seconds >= 10) ? "" : "0") + ((display_seconds > 0) ? to_string(display_seconds) : "0"));
}

Button::Button(Board* b) : board(b) {};

void Button::createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color) {
    rect.setSize(size);
    rect.setPosition(position);
    rect.setFillColor(color);
}

void Button::createSprite(string file) {
    texture = *resource::loadImage(file);
    texture.setSmooth(true);
    sprite.setTexture(texture);
    float scale = BUTTON_SPRITE_SCALE * rect.getSize().y / texture.getSize().y;
    sprite.setScale(sf::Vector2f(scale, scale));
    sprite.setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - (texture.getSize().x * scale) / 2, rect.getPosition().y + rect.getSize().y / 2 - (texture.getSize().y * scale) / 2));
}

bool Button::checkClick(int x, int y) {
    if (x >= rect.getPosition().x && x <= rect.getPosition().x + rect.getSize().x) {
        if (y >= rect.getPosition().y && y <= rect.getPosition().y + rect.getSize().y) {
            return true;
        }
    }

    return false;
}

void Button::click() {}

ButtonResign::ButtonResign(Board* b) : Button::Button(b) {
    createRect(BUTTON_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - BUTTON_SIZE.x / 2 + BUTTON_DISTANCE, TILESIZE * 4 - BUTTON_SIZE.y / 2), BUTTON_RESIGN_COLOR);
    createSprite("resign");
};

void ButtonResign::click() {
    board->appendCPNP("resign");
    board->endGame(resignation, board->me);
}

ButtonDraw::ButtonDraw(Board* b) : Button::Button(b) {
    createRect(BUTTON_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - BUTTON_SIZE.x / 2 - BUTTON_DISTANCE, TILESIZE * 4 - BUTTON_SIZE.y / 2), BUTTON_DRAW_COLOR);
    createSprite("draw");
};

void ButtonDraw::updateState(bool received, bool canceled, bool accepted) {
    if (received) {
        offer_received = true;
        createSprite("v");
    }
    else if (canceled) {
        offer_received = false;
        createSprite("draw");
    }
    else if (accepted) {
        createSprite("draw");
        board->endGame(agreed_draw);
    }
}

void ButtonDraw::click() {
    if (!offer_sent && !offer_received) {
        board->appendCPNP("draw offer");
        createSprite("x");
        offer_sent = true;
    }
    else if (offer_sent) {
        board->appendCPNP("draw cancel");
        createSprite("draw");
        offer_sent = false;
    }
    else if (offer_received) {
        board->appendCPNP("draw accept");
        createSprite("draw");
        board->endGame(agreed_draw);
    }
}

EndScreen::EndScreen(Board* b, GameResult r, string w) : board(b), result(r), winner(w) {
    tie = winner.empty();

    rect.setSize(ENDSCREEN_RECT_SIZE);
    rect.setPosition(sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - ENDSCREEN_RECT_SIZE.x / 2, TILESIZE * 4 - ENDSCREEN_RECT_SIZE.y / 2));
    rect.setFillColor(tie ? ENDSCREEN_BACKGROUND_DRAW : ENDSCREEN_BACKGROUND_WIN);

    font = *resource::loadFont(ENDSCREEN_FONT);

    text[0].setFont(font);
    text[0].setString(tie ? "draw" : winner);
    text[0].setStyle(sf::Text::Bold);

    text[0].setCharacterSize(ENDSCREEN_FONTSIZE_NAME);
    int scaled_font_size = ENDSCREEN_FONTSIZE_NAME;
    if (text[0].getGlobalBounds().width > ENDSCREEN_NAME_MAX_WIDTH) {
        float scale = 1.f * text[0].getGlobalBounds().width / ENDSCREEN_NAME_MAX_WIDTH;
        scaled_font_size = static_cast<int>(ENDSCREEN_FONTSIZE_NAME / scale);
        text[0].setCharacterSize(scaled_font_size);
    }

    text[0].setPosition(sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - text[0].getGlobalBounds().width / 2 + ENDSCREEN_NAME_WIDTH_BUFFER, TILESIZE * 4 - text[0].getGlobalBounds().height / 2 + ENDSCREEN_NAME_HEIGHT));
    text[0].setFillColor(ENDSCREEN_NAME_FILL);

    text[1].setFont(font);
    text[1].setCharacterSize(ENDSCREEN_FONTSIZE_BY);
    text[1].setString(tie ? "by" : "won by");
    text[1].setPosition(sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - text[1].getGlobalBounds().width / 2 + ENDSCREEN_BY_WIDTH_BUFFER, TILESIZE * 4 - text[1].getGlobalBounds().height / 2 + ENDSCREEN_BY_HEIGHT));
    text[1].setFillColor(ENDSCREEN_BY_FILL);
    text[1].setStyle(sf::Text::Bold);

    text[2].setFont(font);
    text[2].setString(stringify(result));
    text[2].setStyle(sf::Text::Bold);

    text[2].setCharacterSize(ENDSCREEN_FONTSIZE_RESULT);
    scaled_font_size = ENDSCREEN_FONTSIZE_RESULT;
    if (text[2].getGlobalBounds().width > ENDSCREEN_RESULT_MAX_WIDTH) {
        float scale = 1.f * text[2].getGlobalBounds().width / ENDSCREEN_RESULT_MAX_WIDTH;
        scaled_font_size = static_cast<int>(ENDSCREEN_FONTSIZE_RESULT / scale);
        text[2].setCharacterSize(scaled_font_size);
    }

    text[2].setPosition(sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - text[2].getGlobalBounds().width / 2 + ENDSCREEN_RESULT_WIDTH_BUFFER, TILESIZE * 4 - text[2].getGlobalBounds().height / 2 + ENDSCREEN_RESULT_HEIGHT));
    text[2].setFillColor(ENDSCREEN_RESULT_FILL);
}

string EndScreen::stringify(GameResult result) {
    if (result == checkmate) return "checkmate";
    else if (result == stalemate) return "stalemate";
    else if (result == resignation) return "resignation";
    else if (result == agreed_draw) return "agreement";
    else if (result == threefold) return "threefold repition";
    else if (result == fifty_moves) return "50 moves";
    else if (result == dead_position) return "dead position";
    else if (result == timeout) return "timeout";
    else if (result == insufficient_material) return "insufficient material";
    else if (result == error_move) return "illegal move";
    else if (result == error_timer) return "timer manipulation";
    else if (result == error_sync) return "synchronization issues";
    else if (result == disconnected) return "disconnected";
    else return "";
}

bool EndScreen::checkClick(int x, int y) {
    if (x >= rect.getPosition().x && x <= rect.getPosition().x + rect.getSize().x) {
        if (y >= rect.getPosition().y && y <= rect.getPosition().y + rect.getSize().y) {
            return true;
        }
    }

    return false;
}

void EndScreen::click() {
    board->running = false;
}