#include "lobby.h"

LobbyButton::LobbyButton(Lobby* l) : lobby(l) {}

void LobbyButton::createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color) {
	rect.setSize(size);
	rect.setPosition(position);
	rect.setFillColor(color);
}

void LobbyButton::createText(string f, int char_size, string t, sf::Vector2f position_offset, sf::Color color, bool bold) {
	sf::Text* text = new sf::Text();
	sf::Font* font = &(*resource::loadFont(f));
	text->setFont(*font);
	text->setCharacterSize(char_size);
	text->setString(t);
	text->setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - text->getLocalBounds().width / 2 + position_offset.x, rect.getPosition().y + rect.getSize().y / 2 - text->getLocalBounds().height / 2 + position_offset.y));
	text->setFillColor(color);
	if (bold) text->setStyle(sf::Text::Bold);

	fonts.push_back(font);
	texts.push_back(text);
}

void LobbyButton::createSprite(string file) {
	texture = *resource::loadImage(file);
	texture.setSmooth(true);
	sprite.setTexture(texture);
	float scale = BUTTON_SPRITE_SCALE * rect.getSize().y / texture.getSize().y;
	sprite.setScale(sf::Vector2f(scale, scale));
	sprite.setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - (texture.getSize().x * scale) / 2, rect.getPosition().y + rect.getSize().y / 2 - (texture.getSize().y * scale) / 2));
}

bool LobbyButton::checkClick(int x, int y) {
	if (x >= rect.getPosition().x && x <= rect.getPosition().x + rect.getSize().x) {
		if (y >= rect.getPosition().y && y <= rect.getPosition().y + rect.getSize().y) {
			return true;
		}
	}

	return false;
}

void LobbyButton::click() {}

LobbyButtonStart::LobbyButtonStart(Lobby* l) : LobbyButton::LobbyButton(l) {
	createRect(LOBBY_START_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_START_SIZE.x / 2, TILESIZE * 4 - LOBBY_START_SIZE.y / 2), LOBBY_START_COLOR);
	createText(LOBBY_START_TEXT_FONT, LOBBY_START_TEXT_FONTSIZE, "START", sf::Vector2f(0, LOBBY_START_TEXT_HEIGHT_OFFSET), LOBBY_START_TEXT_COLOR, true);
	createText(LOBBY_START_SUBTEXT_FONT, LOBBY_START_SUBTEXT_FONTSIZE, "against random", sf::Vector2f(0, LOBBY_START_SUBTEXT_HEIGHT_OFFSET), LOBBY_START_SUBTEXT_COLOR, false);
	has_text = true;
}

void LobbyButtonStart::changeSubtext(string subtext) {
	// implement
}

void LobbyButtonStart::click() {
	cout << "starting game" << endl; // debug
}

LobbyButtonEdit::LobbyButtonEdit(Lobby* l) : LobbyButton::LobbyButton(l) {
	createRect(LOBBY_EDIT_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_EDIT_SIZE.x / 2 + LOBBY_EDIT_OFFSET.x, TILESIZE * 4 - LOBBY_EDIT_SIZE.y / 2 + LOBBY_EDIT_OFFSET.y), LOBBY_EDIT_COLOR);
	createSprite("edit");
	has_text = false;
}

void LobbyButtonEdit::click() {
	cout << "editing" << endl; // debug
}

LobbyInput::LobbyInput(Lobby* l, string p, bool c) : lobby(l), placeholder(p), centered(c) {}

void LobbyInput::createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color) {
	rect.setSize(size);
	rect.setPosition(position);
	rect.setFillColor(color);
}

void LobbyInput::createText(string f, int char_size, sf::Color color, bool bold) {
	font = *resource::loadFont(f);
	text.setFont(font);
	text.setCharacterSize(char_size);
	text.setFillColor(color);
	if (bold) text.setStyle(sf::Text::Bold);

	changeText(placeholder);
}

bool LobbyInput::checkClick(int x, int y) {
	if (x >= rect.getPosition().x && x <= rect.getPosition().x + rect.getSize().x) {
		if (y >= rect.getPosition().y && y <= rect.getPosition().y + rect.getSize().y) {
			return true;
		}
	}

	return false;
}

void LobbyInput::changeText(string new_text) {
	text.setString(new_text);
	text.setPosition(sf::Vector2f(rect.getPosition().x + (centered ? (rect.getSize().x / 2 - text.getLocalBounds().width / 2) : (LOBBY_INPUT_OFFSET)), rect.getPosition().y + rect.getSize().y / 2 - text.getLocalBounds().height * LOBBY_INPUT_TEXT_HEIGHT_OFFSET / 2));
}

void LobbyInput::focus() {
	cout << "focused on " << placeholder << endl; // debug
}

void LobbyInput::unfocus() {
	cout << "unfocused on " << placeholder << endl; // debug
}

LobbySearch::LobbySearch(Lobby* l) : LobbyInput::LobbyInput(l, LOBBY_SEARCH_PLACEHOLDER, false) {
	createRect(LOBBY_SEARCH_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_SEARCH_SIZE.x / 2, TILESIZE * 4 - LOBBY_SEARCH_SIZE.y / 2 + LOBBY_SEARCH_HEIGHT_OIFFSET), LOBBY_SEARCH_COLOR);
	createText(LOBBY_SEARCH_FONT, LOBBY_SEARCH_FONTSIZE, LOBBY_SEARCH_TEXT_COLOR, false);
}

//                                                         to_string(l->node->id)
LobbyName::LobbyName(Lobby* l) : LobbyInput::LobbyInput(l, "omri", true) {
	createRect(LOBBY_NAME_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_NAME_SIZE.x / 2 + LOBBY_NAME_WIDTH_OFFSET, TILESIZE * 4 - LOBBY_NAME_SIZE.y / 2 + LOBBY_NAME_HEIGHT_OFFSET), LOBBY_NAME_COLOR);
	createText(LOBBY_NAME_FONT, LOBBY_NAME_FONTSIZE, LOBBY_NAME_TEXT_COLOR, true);
}