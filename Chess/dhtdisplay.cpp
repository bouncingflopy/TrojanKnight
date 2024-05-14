#include "dhtdisplay.h"

bool display(const DHT& dht, sf::Image& image, sf::Sprite& sprite, sf::Texture& texture) {
    ofstream dotFile("graph.dot");
    dotFile << "graph G {\n";

    for (shared_ptr<DHTConnection> connection : dht.connections) {
        dotFile << "    " << to_string(connection->a->id) << " -- " << to_string(connection->b->id) << ";\n";
    }

    vector<string> colors = { "purple", "green", "red", "blue" };
    for (shared_ptr<DHTNode> node : dht.nodes) {
        if (node->level == -1) {
            dotFile << "    " << to_string(node->id) << "[color = orange];\n";
        }
        else if (node->level <= 3) {
            dotFile << "    " << to_string(node->id) << "[color = " << colors[node->level] << "];\n";
        }
    }
    
    dotFile << "}\n";
    dotFile.close();

    string command = "dot -Tpng graph.dot -o graph.png > NUL 2>&1";
    int result = system(command.c_str());

    if (result == 0) {
        if (image.loadFromFile("graph.png")) {
            texture.loadFromImage(image);
            sprite = sf::Sprite(texture);
            return true;
        }
    }

    return false;
}

void dhtDisplay(const DHT& dht) {
    int old_version = -1;

    shared_ptr<sf::RenderWindow> window;
    sf::Image image;
    sf::Sprite sprite;
    sf::Texture texture;

    while (true) {
        if (old_version != dht.version) {
            if (display(dht, image, sprite, texture)) {
                if (window) window->close();
                window = make_shared<sf::RenderWindow>(sf::VideoMode(image.getSize().x, image.getSize().y), "DHT Graph");
                window->draw(sprite);
                window->display();
            }

            old_version = dht.version;
        }

        if (window) {
            if (window->isOpen()) {
                window->draw(sprite);
                window->display();

                sf::Event event;
                while (window->pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        window->close();
                    }
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(500));
    }
};