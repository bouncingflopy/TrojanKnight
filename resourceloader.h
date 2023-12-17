#ifndef RESOURCELOADER
#define RESOURCELOADER

#include <SFML/Graphics.hpp>
#include <string>

using namespace std;

namespace resource {
	sf::Texture* loadImage(string name);
	sf::Font* loadFont(string name);
}

#endif