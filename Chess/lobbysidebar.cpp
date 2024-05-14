#include "lobby.h"

LobbyButton::LobbyButton() {}

LobbyButton::LobbyButton(Lobby* l) : lobby(l) {}

void LobbyButton::createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color) {
	rect.setSize(size);
	rect.setPosition(position);
	rect.setFillColor(color);
}

bool LobbyButton::checkClick(int x, int y) {
	if (x >= rect.getPosition().x && x <= rect.getPosition().x + rect.getSize().x) {
		if (y >= rect.getPosition().y && y <= rect.getPosition().y + rect.getSize().y) {
			return true;
		}
	}

	return false;
}

void LobbyButton::activate() {}

void LobbyButton::deactivate() {}

void LobbyButton::click() {}

LobbyButtonInvite::LobbyButtonInvite() {}

LobbyButtonInvite::LobbyButtonInvite(Lobby* l, char c) : LobbyButton::LobbyButton(l), action(c) {
	if (action == 'v') initV();
	else if (action == 'x') initX();
}

void LobbyButtonInvite::initV() {
	createRect(LOBBY_INVITE_V_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_INVITE_V_SIZE.x / 2 + LOBBY_INVITE_V_POSITION.x, TILESIZE * 4 - LOBBY_INVITE_V_SIZE.y / 2 + LOBBY_INVITE_V_POSITION.y), LOBBY_INVITE_V_COLOR);
	createSprite("v");
}

void LobbyButtonInvite::initX() {
	createRect(LOBBY_INVITE_X_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_INVITE_X_SIZE.x / 2 + LOBBY_INVITE_X_POSITION.x, TILESIZE * 4 - LOBBY_INVITE_X_SIZE.y / 2 + LOBBY_INVITE_X_POSITION.y), LOBBY_INVITE_X_COLOR);
	createSprite("x");
}

void LobbyButtonInvite::createSprite(string file) {
	texture = *resource::loadImage(file);
	texture.setSmooth(true);
	sprite.setTexture(texture);
	float scale = BUTTON_SPRITE_SCALE * rect.getSize().y / texture.getSize().y;
	sprite.setScale(sf::Vector2f(scale, scale));
	sprite.setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - (texture.getSize().x * scale) / 2, rect.getPosition().y + rect.getSize().y / 2 - (texture.getSize().y * scale) / 2));
}

void LobbyButtonInvite::click() {
	if (action == 'v') lobby->acceptInvite();
	else if (action == 'x') lobby->rejectInvite();
}

LobbyButtonStart::LobbyButtonStart(Lobby* l) : LobbyButton::LobbyButton(l) {
	createRect(LOBBY_START_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_START_SIZE.x / 2, TILESIZE * 4 - LOBBY_START_SIZE.y / 2), LOBBY_START_UNCLICKABLE_COLOR);
	
	font = *resource::loadFont(LOBBY_START_FONT);
	createText(text, LOBBY_START_TEXT_FONTSIZE, "START", LOBBY_START_TEXT_HEIGHT_OFFSET, LOBBY_START_TEXT_COLOR, true);
	createText(subtext, LOBBY_START_SUBTEXT_FONTSIZE, LOBBY_START_PLACEHOLDER, LOBBY_START_SUBTEXT_HEIGHT_OFFSET, LOBBY_START_SUBTEXT_COLOR, false);
}

void LobbyButtonStart::createText(sf::Text& current_text, int font_size, string t, int height_offset, sf::Color color, bool bold) {
	current_text.setFont(font);
	current_text.setCharacterSize(font_size);
	current_text.setString(t);
	current_text.setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - current_text.getGlobalBounds().width / 2, rect.getPosition().y + rect.getSize().y / 2 - current_text.getGlobalBounds().height / 2 + height_offset));
	current_text.setFillColor(color);
	if (bold) current_text.setStyle(sf::Text::Bold);
}

void LobbyButtonStart::changeText(string new_text) {
	text.setString(new_text);
	text.setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - text.getGlobalBounds().width / 2, rect.getPosition().y + rect.getSize().y / 2 - text.getGlobalBounds().height / 2 + LOBBY_START_TEXT_HEIGHT_OFFSET));
}

void LobbyButtonStart::changeSubtext(string new_subtext) {
	subtext.setString(new_subtext);
	subtext.setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - subtext.getGlobalBounds().width / 2, rect.getPosition().y + rect.getSize().y / 2 - subtext.getGlobalBounds().height / 2 + LOBBY_START_SUBTEXT_HEIGHT_OFFSET));
}

void LobbyButtonStart::makeUnclickable() {
	clickable = false;
	rect.setFillColor(LOBBY_START_UNCLICKABLE_COLOR);
}

void LobbyButtonStart::makeClickable() {
	clickable = true;
	rect.setFillColor(LOBBY_START_CLICKABLE_COLOR);
}

void LobbyButtonStart::activate() {
	activated = true;

	rect.setFillColor(LOBBY_START_ACTIVE_COLOR);
	changeText("CANCEL");

	lobby->node->sendInvite(lobby->selected_tile->lobby_player->id);
}

void LobbyButtonStart::deactivate() {
	activated = false;

	rect.setFillColor(LOBBY_START_CLICKABLE_COLOR);
	changeText("START");

	lobby->node->cancelInvite();
}

void LobbyButtonStart::click() {
	if (!activated) activate();
	else deactivate();
}

LobbyButtonEdit::LobbyButtonEdit(Lobby* l, LobbyInput* i) : LobbyButton::LobbyButton(l), input((LobbyName*)i) {
	createRect(LOBBY_EDIT_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_EDIT_SIZE.x / 2 + LOBBY_EDIT_OFFSET.x, TILESIZE * 4 - LOBBY_EDIT_SIZE.y / 2 + LOBBY_EDIT_OFFSET.y), LOBBY_EDIT_DEACTIVE_COLOR);
	createSprite("edit");
}

void LobbyButtonEdit::createSprite(string file) {
	texture = *resource::loadImage(file);
	texture.setSmooth(true);
	sprite.setTexture(texture);
	float scale = BUTTON_SPRITE_SCALE * rect.getSize().y / texture.getSize().y;
	sprite.setScale(sf::Vector2f(scale, scale));
	sprite.setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - (texture.getSize().x * scale) / 2, rect.getPosition().y + rect.getSize().y / 2 - (texture.getSize().y * scale) / 2));
}

void LobbyButtonEdit::activate() {
	activated = true;

	rect.setFillColor(LOBBY_EDIT_ACTIVE_COLOR);
	createSprite("v");

	input->focus();
}

void LobbyButtonEdit::deactivate() {
	activated = false;

	rect.setFillColor(LOBBY_EDIT_DEACTIVE_COLOR);
	createSprite("edit");

	input->unfocus();
}

void LobbyButtonEdit::click() {
	if (!activated) activate();
	else {
		if (!input->value.empty()) {
			if (lobby->dht.checkNameFree(input->value)) {
				input->name = input->value;
				lobby->node->changeName(input->name);
			}
		}

		deactivate();
	}
}

LobbyInput::LobbyInput(Lobby* l, bool c) : lobby(l), centered(c) {}

void LobbyInput::createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color) {
	rect.setSize(size);
	rect.setPosition(position);
	rect.setFillColor(color);
}

void LobbyInput::createText(string f, sf::Color color, bool bold) {
	font = *resource::loadFont(f);
	text.setFont(font);
	text.setFillColor(color);
	if (bold) text.setStyle(sf::Text::Bold);
}

bool LobbyInput::checkClick(int x, int y) {
	if (x >= rect.getPosition().x && x <= rect.getPosition().x + rect.getSize().x) {
		if (y >= rect.getPosition().y && y <= rect.getPosition().y + rect.getSize().y) {
			return true;
		}
	}

	return false;
}

void LobbyInput::displayText(string display) {
	text.setString(display + (focused ? "|" : ""));
	
	text.setCharacterSize(font_size);
	int scaled_font_size = font_size;
	if (text.getGlobalBounds().width > max_width) {
		float scale = 1.f * text.getGlobalBounds().width / max_width;
		scaled_font_size = static_cast<int>(font_size / scale);
		text.setCharacterSize(scaled_font_size);
	}

	text.setPosition(sf::Vector2f(rect.getPosition().x + (centered ? (rect.getSize().x / 2 - text.getGlobalBounds().width / 2) : (LOBBY_INPUT_PADDING)), rect.getPosition().y + rect.getSize().y / 2 - sf::Text("x", font, scaled_font_size).getLocalBounds().height * height_offset));
}

void LobbyInput::handleEnter() {
	unfocus();
}

void LobbyInput::handleUnicode(uint32_t unicode) {
	if (!(unicode == '\b' || unicode == '\n' ||
		unicode >= 'a' && unicode <= 'z' ||
		unicode >= 'A' && unicode <= 'Z' ||
		unicode >= '0' && unicode <= '9')) return;

	if (unicode == '\b') {
		if (!value.empty()) {
			value.pop_back();
			changedText();
		}
	}
	else if (unicode == '\n') {
		handleEnter();
	}
	else {
		if (value.size() < max_length) {
			value += static_cast<char>(unicode);
			changedText();
		}
	}
}

void LobbyInput::changedText() {
	displayText(value);
}

void LobbyInput::focus() {
	lobby->active_input = this;
	focused = true;
	displayText(value);
}

void LobbyInput::unfocus() {
	focused = false;
	lobby->active_input = nullptr;
}

LobbySearch::LobbySearch(Lobby* l) : LobbyInput::LobbyInput(l, false) {
	font_size = LOBBY_SEARCH_FONTSIZE;
	max_width = LOBBY_SEARCH_TEXT_MAX_WIDTH;
	height_offset = LOBBY_SEARCH_TEXT_HEIGHT_OFFSET;
	placeholder = LOBBY_SEARCH_PLACEHOLDER;

	createRect(LOBBY_SEARCH_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_SEARCH_SIZE.x / 2, TILESIZE * 4 - LOBBY_SEARCH_SIZE.y / 2 + LOBBY_SEARCH_HEIGHT_OFFSET), LOBBY_SEARCH_COLOR);
	createText(LOBBY_SEARCH_FONT, LOBBY_SEARCH_TEXT_COLOR, false);

	displayText(placeholder);
}

void LobbySearch::changedText() {
	LobbyInput::changedText();
	lobby->applyFilter(value);
}

void LobbySearch::unfocus() {
	LobbyInput::unfocus();

	if (value.empty()) displayText(placeholder);
	else displayText(value);
}

LobbyName::LobbyName(Lobby* l) : LobbyInput::LobbyInput(l, true) {
	font_size = LOBBY_NAME_FONTSIZE;
	max_width = LOBBY_NAME_TEXT_MAX_WIDTH;
	height_offset = LOBBY_NAME_TEXT_HEIGHT_OFFSET;
	name = to_string(lobby->node->id); // implement loading from appdata

	createRect(LOBBY_NAME_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_NAME_SIZE.x / 2 + LOBBY_NAME_WIDTH_OFFSET, TILESIZE * 4 - LOBBY_NAME_SIZE.y / 2 + LOBBY_NAME_HEIGHT_OFFSET), LOBBY_NAME_COLOR);
	createText(LOBBY_NAME_FONT, LOBBY_NAME_TEXT_COLOR, true);

	displayText(name);
}

void LobbyName::handleEnter() {
	edit->click();
}

void LobbyName::unfocus() {
	if (edit->activated) {
		edit->deactivate();
		return;
	}

	LobbyInput::unfocus();

	displayText(name);
}

LobbyInvite::LobbyInvite(Lobby* l, string name) : lobby(l) {
	createRect(LOBBY_INVITE_SIZE, sf::Vector2f(TILESIZE * 8 + SIDEBAR / 2 - LOBBY_INVITE_SIZE.x / 2 + LOBBY_INVITE_POSITION.x, TILESIZE * 4 - LOBBY_INVITE_SIZE.y / 2 + LOBBY_INVITE_POSITION.y), LOBBY_INVITE_COLOR);

	font = *resource::loadFont(LOBBY_INVITE_FONT);
	createText(text, LOBBY_INVITE_TEXT_FONTSIZE, "INVITE FROM", LOBBY_INVITE_TEXT_OFFSET, LOBBY_INVITE_TEXT_MAX_WIDTH, LOBBY_INVITE_TEXT_COLOR, true);
	createText(subtext, LOBBY_INVITE_SUBTEXT_FONTSIZE, name, LOBBY_INVITE_SUBTEXT_OFFSET, LOBBY_INVITE_SUBTEXT_MAX_WIDTH, LOBBY_INVITE_SUBTEXT_COLOR, true);

	button_v = new LobbyButtonInvite(lobby, 'v');
	button_x = new LobbyButtonInvite(lobby, 'x');
}

void LobbyInvite::createRect(sf::Vector2f size, sf::Vector2f position, sf::Color color) {
	rect.setSize(size);
	rect.setPosition(position);
	rect.setFillColor(color);
}

void LobbyInvite::createText(sf::Text& current_text, int font_size, string t, sf::Vector2f position_offset, int max_width, sf::Color color, bool bold) {
	current_text.setFont(font);
	current_text.setString(t);

	current_text.setCharacterSize(font_size);
	int scaled_font_size = font_size;
	if (current_text.getGlobalBounds().width > max_width) {
		float scale = 1.f * current_text.getGlobalBounds().width / max_width;
		scaled_font_size = static_cast<int>(font_size / scale);
		current_text.setCharacterSize(scaled_font_size);
	}

	current_text.setPosition(sf::Vector2f(rect.getPosition().x + rect.getSize().x / 2 - current_text.getGlobalBounds().width / 2 + position_offset.x, rect.getPosition().y + rect.getSize().y / 2 - current_text.getGlobalBounds().height / 2 + position_offset.y));
	current_text.setFillColor(color);
	if (bold) current_text.setStyle(sf::Text::Bold);
}