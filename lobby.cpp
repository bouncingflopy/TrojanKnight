#include "lobby.h"

Lobby::Lobby(Node* n) : node(n) {
	tiles = createTiles();
	createArrows();

	input_search = new LobbySearch(this);
	input_name = new LobbyName(this);
	button_start = new LobbyButtonStart(this);
	button_edit = new LobbyButtonEdit(this, input_name);
	input_name->edit = button_edit;

	dht = DHT();
	update();
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

LobbyTile*** Lobby::createTiles() {
	LobbyTile*** tiles = new LobbyTile * *[8];

	for (int i = 0; i < 8; i++) {
		tiles[i] = new LobbyTile * [8];
		for (int j = 0; j < 8; j++) {
			tiles[i][j] = new LobbyTile(i, j);
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
	
	window.draw(button_start->rect);
	window.draw(button_start->text);
	window.draw(button_start->subtext);

	window.draw(button_edit->rect);
	window.draw(button_edit->sprite);

	window.draw(input_search->rect);
	window.draw(input_search->text);

	window.draw(input_name->rect);
	window.draw(input_name->text);

	if (lobby_invite) {
		window.draw(lobby_invite->rect);
		window.draw(lobby_invite->text);
		window.draw(lobby_invite->subtext);
		window.draw(lobby_invite->button_v->rect);
		window.draw(lobby_invite->button_v->sprite);
		window.draw(lobby_invite->button_x->rect);
		window.draw(lobby_invite->button_x->sprite);
	}
}

// debug
#include <random>
string generateRandomString(int length) {random_device rd;mt19937 gen(rd());uniform_int_distribution<> disUppercase('A', 'Z');uniform_int_distribution<> disLowercase('a', 'z');string randomString;randomString.reserve(length);for (int i = 0; i < length; ++i) {if (std::uniform_int_distribution<>{0, 1}(gen) == 0) {randomString.push_back(static_cast<char>(disUppercase(gen)));}else {randomString.push_back(static_cast<char>(disLowercase(gen)));}}return randomString;}

void Lobby::update() {
	if (!(node->dht == dht)) {
		dht = node->dht;
		
		/*// debug
		lobby_players.clear();
		for (int i = 0; i < 256; i++) {
			lobby_players.push_back(new LobbyPlayer(i, generateRandomString(MAX_NAME_LENGTH)));
		}*/

		lobby_players.clear();
		
		vector<shared_ptr<DHTNode>> copied_dht_nodes;
		dht.copyNodes(copied_dht_nodes);

		for (shared_ptr<DHTNode>& dht_node : copied_dht_nodes) {
			if (dht_node->level > -1) {
				lobby_players.push_back(new LobbyPlayer(dht_node->id, dht_node->name));
			}
		}

		applyFilter(filter);

		if (filtered_lobby_players.size() - (page * 62 + 1) <= 1) {
			page--;
			if (selected_tile) deselectTile(selected_tile);
		}
		openPage(page);

		if (selected_tile) {
			LobbyTile* copied_selected_tile = selected_tile;
			deselectTile(selected_tile);
			selectTile(copied_selected_tile);
		}
	}

	if (node->incoming_invites.size() > 0 && (!incoming_invite || node->incoming_invites[0].get() != incoming_invite.get())) {
		incoming_invite.reset();
		lobby_invite.reset();
		incoming_invite = node->incoming_invites[0];
		displayInvite();
	}
	else if (incoming_invite && node->incoming_invites.size() == 0) {
		incoming_invite.reset();
		lobby_invite.reset();
	}

	if (button_start->activated && !node->outgoing_invite) {
		button_start->deactivate();
	}

	if (node->chess_connection) running = false;
}

void Lobby::applyFilter(string expression) {
	filter = expression;

	string lowercase_expression = expression;
	transform(lowercase_expression.begin(), lowercase_expression.end(), lowercase_expression.begin(), [](unsigned char c) {return tolower(c);});

	filtered_lobby_players.clear();
	for (LobbyPlayer* lobby_player : lobby_players) {
		string lowercase_name = lobby_player->name;
		transform(lowercase_name.begin(), lowercase_name.end(), lowercase_name.begin(), [](unsigned char c) {return tolower(c);});

		if (lowercase_name.find(lowercase_expression) != string::npos) {
			filtered_lobby_players.push_back(lobby_player);
		}
	}

	openPage(0);
}

void Lobby::openPage(int new_page) {
	previous = false;
	next = false;

	for (LobbyPlayer* lobby_player : page_lobby_players) {
		lobby_player->piece->tile->lobby_player = nullptr;
	}

	if (filtered_lobby_players.size() <= 64) {
		page_lobby_players = filtered_lobby_players;

		for (int i = 0; i < page_lobby_players.size(); i++) {
			page_lobby_players[i]->moveTo(tiles[i / 8][i % 8]);
		}
	}
	else if (new_page == 0) {
		page_lobby_players = vector<LobbyPlayer*>(filtered_lobby_players.begin(), filtered_lobby_players.begin() + 63);
		next = true;

		for (int i = 0; i < 63; i++) {
			page_lobby_players[i]->moveTo(tiles[i / 8][i % 8]);
		}
	}
	else if (filtered_lobby_players.size() - (new_page * 62 + 1) <= 63) {
		page_lobby_players = vector<LobbyPlayer*>(filtered_lobby_players.begin() + new_page * 62 + 1, filtered_lobby_players.end());
		previous = true;

		int i;
		for (i = 0; i < 56 && i < filtered_lobby_players.size() - (new_page * 62 + 1); i++) {
			page_lobby_players[i]->moveTo(tiles[i / 8][i % 8]);
		}
		for (int j = 1; j < 8 && i < filtered_lobby_players.size() - (new_page * 62 + 1); j++) {
			page_lobby_players[i]->moveTo(tiles[7][j]);
			i++;
		}
	}
	else {
		page_lobby_players = vector<LobbyPlayer*>(filtered_lobby_players.begin() + new_page * 62 + 1, filtered_lobby_players.begin() + (new_page + 1) * 62 + 1);
		next = true;
		previous = true;

		int i;
		for (i = 0; i < 56; i++) {
			page_lobby_players[i]->moveTo(tiles[i / 8][i % 8]);
		}
		for (int j = 1; j < 7; j++) {
			page_lobby_players[i]->moveTo(tiles[7][j]);
			i++;
		}
	}
}

void Lobby::displayInvite() {
	lobby_invite = make_shared<LobbyInvite>(this, dht.getNodeFromId(incoming_invite->from)->name);
}

void Lobby::acceptInvite() {
	node->acceptInvite();
	running = false;
}

void Lobby::rejectInvite() {
	node->rejectInvite();
	lobby_invite.reset();
}

void Lobby::selectTile(LobbyTile* tile) {
	tile->select();
	selected_tile = tile;

	button_start->changeSubtext("against " + tile->lobby_player->name);
	button_start->makeClickable();
}

void Lobby::deselectTile(LobbyTile* tile) {
	tile->deselect();
	selected_tile = nullptr;

	button_start->changeSubtext(LOBBY_START_PLACEHOLDER);
	button_start->makeUnclickable();
}

void Lobby::handlePress(int x, int y) {
	if (button_start->activated) {
		if (button_start->checkClick(x, y)) button_start->click();
		return;
	}

	if (active_input != nullptr) { // change to shared_ptr
		if (!active_input->checkClick(x, y)) {
			if (!(active_input == input_name && button_edit->checkClick(x, y))) active_input->unfocus();
		}
	}

	if (x <= TILESIZE * 8 && y <= TILESIZE * 8) {
		LobbyTile* pressed = tiles[y / TILESIZE][x / TILESIZE];

		if (!selected_tile) {
			if (pressed->lobby_player) {
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
			else if (pressed->lobby_player) {
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
			if (input_search->checkClick(x, y)) {
				input_search->focus();
				if (selected_tile) deselectTile(selected_tile);
			}
		}

		if (button_start->clickable && button_start->checkClick(x, y)) button_start->click();
		if (button_edit->checkClick(x, y)) button_edit->click();
		if (lobby_invite) {
			if (lobby_invite->button_v->checkClick(x, y)) lobby_invite->button_v->click();
			if (lobby_invite->button_x->checkClick(x, y)) lobby_invite->button_x->click();
		}
	}
}