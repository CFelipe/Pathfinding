#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "ResourcePath.hpp"
#include <list>
#include <string>
#include <iostream>

const sf::Color dark = sf::Color(51, 51, 51, 255);
const sf::Color borderColor = sf::Color(102, 102, 102, 255);
const sf::Color gridBg = sf::Color(242, 242, 242, 255);

class RadioOption : public sf::Transformable, public sf::Drawable {
public:
    RadioOption(const sf::String& text, const sf::Font& font) {
        label = sf::Text(text, font, 26);
        label.setColor(dark);
    }
    
    sf::Text label;
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(label, states);
    }
};

class RadioGroup : public sf::Transformable, public sf::Drawable {
public:
    RadioGroup(unsigned int width)
    : width(width)
    {
        topLine = sf::RectangleShape(sf::Vector2f(width, 2));
        topLine.setFillColor(dark);
        bottomLine = sf::RectangleShape(sf::Vector2f(width, 2));
        bottomLine.setFillColor(dark);
    }
    
    void addOption(RadioOption option) {
        option.setPosition(0, 8 + options.size() * 34);
        options.push_back(option);
        bottomLine.move(0, 34 + 8);
    }
    
    unsigned int getHeight() {
        return bottomLine.getPosition().y - topLine.getPosition().y + 4;
    }
    
    RadioOption* selectedOption;
    std::list<RadioOption> options;
    
    unsigned int width;
    sf::RectangleShape topLine;
    sf::RectangleShape bottomLine;

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(topLine, states);
        
        for(RadioOption option : options) {
            target.draw(option, states);
        }
        
        target.draw(bottomLine, states);
    }
};

class Button : public sf::Transformable, public sf::Drawable {
public:
    Button(const sf::String& text, unsigned int width, const sf::Font& font)
    : width(width)
    {
        borderRect = sf::RectangleShape(sf::Vector2f(width, 50));
        borderRect.setFillColor(dark);
        
        rect = sf::RectangleShape(sf::Vector2f(width - 4, 50 - 4));
        rect.setPosition(2, 2);
        rect.setFillColor(sf::Color::White);
        
        label = sf::Text(text, font, 26);
        label.setColor(dark);
        
        sf::FloatRect textRect = label.getLocalBounds();
        sf::FloatRect rectBounds = rect.getLocalBounds();
        
        label.setOrigin((int) (textRect.left + textRect.width / 2.0f),
                        (int) (textRect.top  + textRect.height / 2.0f));
        
        label.setPosition((int) (rectBounds.left + rectBounds.width / 2),
                          (int) (rectBounds.top + rectBounds.height / 2) + 2);
    }
    
    unsigned int width;
    sf::RectangleShape borderRect;
    sf::RectangleShape rect;
    sf::Text label;
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(borderRect, states);
        target.draw(rect, states);
        target.draw(label, states);
    }
};

class Slider : public sf::Transformable, public sf::Drawable {
public:
    Slider(unsigned int maxValue, unsigned int width)
    : maxValue(maxValue),
      width(width)
    {
        line = sf::RectangleShape(sf::Vector2f(width, 2));
        line.setFillColor(dark);
        line.setPosition(0, 6);
        handleBorder = sf::CircleShape(7);
        handleBorder.setFillColor(dark);
        handle = sf::CircleShape(5);
        handle.setFillColor(sf::Color::White);
        handle.setPosition(2, 2);
    }
    
    unsigned int maxValue;
    
    unsigned int width;
    sf::IntRect clickableArea;
    sf::CircleShape handle;
    sf::CircleShape handleBorder;
    sf::RectangleShape line;
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(line, states);
        target.draw(handleBorder, states);
        target.draw(handle, states);
    }
};

class Node : public sf::Transformable, public sf::Drawable {
public:
    Node(unsigned int width, unsigned int height) {
        rect = sf::RectangleShape(sf::Vector2f(width, height));
        rect.setFillColor(gridBg);
    }
    
private:
    sf::RectangleShape rect;
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(rect, states);
    }
};

class Grid : public sf::Transformable, public sf::Drawable {
public:
    Grid(unsigned int rows, unsigned int columns, unsigned int width, unsigned int height)
    : rows(rows),
      columns(columns),
      width(width),
      height(height)
    {
        borderRect = sf::RectangleShape(sf::Vector2f(width + 1, height + 1));
        borderRect.setFillColor(borderColor);
        
        unsigned int nodeW = (width / columns) - 1;
        unsigned int nodeH = (height / rows) - 1;
        
        for(int i = 0; i < columns; ++i) {
            for(int j = 0; j < rows; ++j) {
                Node node(nodeW, nodeH);
                node.setPosition(1 + ((nodeW + 1) * i), 1 + ((nodeH + 1) * j));
                nodes.push_back(node);
            }
        }
    }
    
    std::list<Node> nodes;
    
private:
    unsigned int rows;
    unsigned int columns;
    unsigned int width;
    unsigned int height;
    sf::RectangleShape borderRect;
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(borderRect, states);
        
        for(Node node : nodes) {
            target.draw(node, states);
        }
    }
};

int main(int, char const**) {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    
    sf::RenderWindow window(sf::VideoMode(800, 600), "Pathfinding", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile(resourcePath() + "inconsolata.otf")) {
        return EXIT_FAILURE;
    }
    
    unsigned int xSpace = 58;
    unsigned int ySpace = 58;

    // Add GUI elements ---
    RadioGroup radioGroup(162);
    radioGroup.setPosition(xSpace, ySpace);
    RadioOption aStar(sf::String(L"A*"), font);
    radioGroup.addOption(aStar);
    RadioOption greedy(sf::String(L"Gulosa"), font);
    radioGroup.addOption(greedy);
    ySpace += radioGroup.getHeight() + 12;
    
    Button cleanButton(sf::String(L"Limpar"), 162, font);
    cleanButton.setPosition(xSpace, ySpace);
    ySpace += 50 + 12;
    
    Button iterateButton(sf::String(L">"), 46, font);
    iterateButton.setPosition(xSpace, ySpace);
    xSpace += 12 + 46;
    
    Button rewindButton(sf::String(L"<"), 46, font);
    rewindButton.setPosition(xSpace, ySpace);
    xSpace += 12 + 46;
    
    Button endButton(sf::String(L">>"), 46, font);
    endButton.setPosition(xSpace, ySpace);
    ySpace += 50 + 12;
    xSpace = 58;
    
    Slider slider(255, 162);
    slider.setPosition(xSpace, ySpace);
    xSpace += 162 + 36;
    ySpace = 58;
    
    Grid grid(15, 15, 480, 480);
    grid.setPosition(xSpace, ySpace);
    
    // ------------------------
    
    while(window.isOpen()) {
        sf::Event event;
        
        while(window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }
        
        window.clear(sf::Color::White);

        window.draw(radioGroup);
        window.draw(cleanButton);
        window.draw(iterateButton);
        window.draw(rewindButton);
        window.draw(endButton);
        window.draw(slider);
        window.draw(grid);
        
        window.display();
    }

    return EXIT_SUCCESS;
}
