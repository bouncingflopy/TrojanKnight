#include "lobby.h"

Lobby::Lobby(Node* n) : node(n) {
	player = new Player(0, "", 0);
	tiles = createTiles();
	createArrows();

	buttons.push_back(new LobbyButtonStart(this));
	buttons.push_back(new LobbyButtonEdit(this));
	inputs.push_back(new LobbySearch(this));
	inputs.push_back(new LobbyName(this));

	dht = DHT();
	updateDHT();
}

void Lobby::createArrows() {
	previous_texture = *resource::loadImage("previous_arrow");
	previous_texture.setSmooth(true);
	previous_sprite.setTexture(previous_texture);
	float scale = LOBBY_ARROW_SCALE * TILESIZE / previous_texture.getSize().x;
	previous_sprite.setScale(sf::Vector2f(scale, scale));
	previous_sprite.setPosition(sf::Vector2f(TILESIZE / 2 - previous_sprite.getLocalBounds().width * scale / 2, TILESIZE * 8 - TILESIZE / 2 - previous_sprite.getLocalBounds().height * scale / 2));

	next_texture = *resource::loadImage("next_arrow");
	next_texture.setSmooth(true);
	next_sprite.setTexture(next_texture);
	scale = LOBBY_ARROW_SCALE * TILESIZE / next_texture.getSize().x;
	next_sprite.setScale(sf::Vector2f(scale, scale));
	next_sprite.setPosition(sf::Vector2f(TILESIZE * 8 - TILESIZE / 2 - next_sprite.getLocalBounds().width * scale / 2, TILESIZE * 8 - TILESIZE / 2 - next_sprite.getLocalBounds().height * scale / 2));
}

Tile*** Lobby::createTiles() {
	Tile*** tiles = new Tile * *[8];

	for (int i = 0; i < 8; i++) {
		tiles[i] = new Tile * [8];
		for (int j = 0; j < 8; j++) {
			tiles[i][j] = new Tile(i, j, 1);
		}
	}

	return tiles;
}

void Lobby::draw(sf::RenderWindow& window) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			window.draw(tiles[i][j]->rect);
		}
	}

	for (LobbyPlayer* lobby_player : page_lobby_players) {
		window.draw(lobby_player->piece->sprite);
		window.draw(lobby_player->rect);
		window.draw(lobby_player->text);
	}

	if (previous) window.draw(previous_sprite);
	if (next) window.draw(next_sprite);

	for (LobbyButton* button : buttons) {
		window.draw(button->rect);
		if (button->has_text) {
			for (sf::Text* text : button->texts) {
				window.draw(*text);
			}
		}
		else window.draw(button->sprite);
	}

	for (LobbyInput* input : inputs) {
		window.draw(input->rect);
		window.draw(input->text);
	}
}

// debug
#include <random>
string generateRandomString(int length) {random_device rd;mt19937 gen(rd());uniform_int_distribution<> disUppercase('A', 'Z');uniform_int_distribution<> disLowercase('a', 'z');string randomString;randomString.reserve(length);for (int i = 0; i < length; ++i) {if (std::uniform_int_distribution<>{0, 1}(gen) == 0) {randomString.push_back(static_cast<char>(disUppercase(gen)));}else {randomString.push_back(static_cast<char>(disLowercase(gen)));}}return randomString;}

void Lobby::updateDHT() {
	if (!(node->dht == dht)) {
		// implement updating lobby players
		
		// debug
		lobby_players.clear();
		for (int i = 0; i < 256; i++) {
			lobby_players.push_back(new LobbyPlayer(i, to_string(i)));
		}

		dht = node->dht;

		applyFilter("");
	}
}

void Lobby::applyFilter(string expression) {
	regex pattern(".*" + expression + ".*", regex_constants::icase);

	filtered_lobby_players.clear();
	for (LobbyPlayer* lobby_player : lobby_players) {
		if (regex_match(lobby_player->name, pattern)) {
			filtered_lobby_players.push_back(lobby_player);
		}
	}

	openPage(0);
}

void Lobby::openPage(int new_page) {
	previous = false;
	next = false;

	for (LobbyPlayer* lobby_player : page_lobby_players) {
		lobby_player->piece->tile->piece = NULL;
	}

	if (filtered_lobby_players.size() <= 64) {
		page_lobby_players = filtered_lobby_players;

		for (int i = 0; i < page_lobby_players.size(); i++) {
			page_lobby_players[i]->moveTo(tiles[i / 8][i % 8], player);
		}
	}
	else if (new_page == 0) {
		page_lobby_players = vector<LobbyPlayer*>(filtered_lobby_players.begin(), filtered_lobby_players.begin() + 63);
		next = true;

		for (int i = 0; i < 63; i++) {
			page_lobby_players[i]->moveTo(tiles[i / 8][i % 8], player);
		}
	}
	else if (filtered_lobby_players.size() - (new_page * 62 + 1) <= 63) {
		page_lobby_players = vector<LobbyPlayer*>(filtered_lobby_players.begin() + new_page * 62 + 1, filtered_lobby_players.end());
		previous = true;

		int i;
		for (i = 0; i < 56 && i < filtered_lobby_players.size() - (new_page * 62 + 1); i++) {
			page_lobby_players[i]->moveTo(tiles[i / 8][i % 8], player);
		}
		for (int j = 1; j < 8 && i < filtered_lobby_players.size() - (new_page * 62 + 1); j++) {
			page_lobby_players[i]->moveTo(tiles[7][j], player);
			i++;
		}
	}
	else {
		page_lobby_players = vector<LobbyPlayer*>(filtered_lobby_players.begin() + new_page * 62 + 1, filtered_lobby_players.begin() + (new_page + 1) * 62 + 1);
		next = true;
		previous = true;

		int i;
		for (i = 0; i < 56; i++) {
			page_lobby_players[i]->moveTo(tiles[i / 8][i % 8], player);
		}
		for (int j = 1; j < 7; j++) {
			page_lobby_players[i]->moveTo(tiles[7][j], player);
			i++;
		}
	}
}

void Lobby::selectTile(Tile* tile) {
	tile->select();
	selected_tile = tile;
}

void Lobby::deselectTile(Tile* tile) {
	tile->deselect();
	selected_tile = NULL;
}

void Lobby::handlePress(int x, int y) {
	if (active_input != nullptr) { // change to shared_ptr
		if (!active_input->checkClick(x, y)) {
			active_input->unfocus();;
			active_input = nullptr;
		}
	}

	if (x <= TILESIZE * 8 && y <= TILESIZE * 8) {
		Tile* pressed = tiles[y / TILESIZE][x / TILESIZE];

		if (!selected_tile) {
			if (pressed->piece) {
				selectTile(pressed);
			}
			else if (previous && pressed->row == 7 && pressed->column == 0) {
				page--;
				openPage(page);
			}
			else if (next && pressed->row == 7 && pressed->column == 7) {
				page++;
				openPage(page);
			}
		}
		else {
			if (pressed->selected) {
				deselectTile(selected_tile);
			}
			else if (pressed->piece) {
				deselectTile(selected_tile);
				selectTile(pressed);
			}
			else if (previous && pressed->row == 7 && pressed->column == 0) {
				deselectTile(selected_tile);
				page--;
				openPage(page);
			}
			else if (next && pressed->row == 7 && pressed->column == 7) {
				deselectTile(selected_tile);
				page++;
				openPage(page);
			}
			else {
				deselectTile(selected_tile);
			}
		}
	}
	else {
		if (active_input == nullptr) { // change to shared_ptr
			for (LobbyInput* input : inputs) {
				if (input->checkClick(x, y)) {
					active_input = input;
					active_input->focus();
					if (selected_tile) deselectTile(selected_tile);
				}
			}
		}

		for (LobbyButton* button : buttons) {
			if (button->checkClick(x, y)) button->click();
		}
	}
}