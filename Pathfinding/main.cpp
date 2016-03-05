#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "ResourcePath.hpp"

int main(int, char const**) {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Pathfinding");

    sf::Font font;
    if (!font.loadFromFile(resourcePath() + "inconsolata.otf")) {
        return EXIT_FAILURE;
    }
    
    sf::Text text("Hello", font, 50);
    text.setColor(sf::Color::Black);

    while(window.isOpen()) {
        sf::Event event;
        
        while(window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }
        
        window.clear(sf::Color::White);

        window.draw(text);

        window.display();
    }

    return EXIT_SUCCESS;
}
