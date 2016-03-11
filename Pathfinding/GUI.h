//
//  GUI.h
//  Pathfinding
//
//  Created by Felipe on 3/10/16.
//  Copyright (c) 2016 Felipe. All rights reserved.
//

#ifndef Pathfinding_GUI_h
#define Pathfinding_GUI_h

const sf::Color dark = sf::Color(51, 51, 51, 255);
const sf::Color borderColor = sf::Color(102, 102, 102, 255);
const sf::Color gridBg = sf::Color(242, 242, 242, 255);
const sf::Color wallColor = sf::Color(165, 165, 165, 255);
const sf::Color startGreen = sf::Color(89, 198, 95, 255);
const sf::Color goalRed = sf::Color(196, 91, 91, 255);
const sf::Color visitedBlue = sf::Color(145, 194, 216, 255);

class RadioOption : public sf::Transformable, public sf::Drawable {
public:
    RadioOption(const sf::String& text, const sf::Font& font, sf::Transformable* parent) {
        this->parent = parent;
        
        label = sf::Text(text, font, 26);
        label.setColor(dark);
        label.setPosition(30, 0);
        
        circleBorder = sf::CircleShape(7);
        circleBorder.setFillColor(dark);
        circleBorder.setPosition(0, 10);
        circle = sf::CircleShape(5);
        circle.setFillColor(sf::Color::White);
        circle.setPosition(2, 12);
        
        clickArea = sf::FloatRect(0, 0,
                                  label.getPosition().x + label.getLocalBounds().width,
                                  30);
    }
    
    sf::FloatRect clickArea;
    sf::Text label;
    sf::CircleShape circleBorder;
    sf::CircleShape circle;
    sf::Transformable* parent;
    
    bool contains(sf::Vector2i point) {
        return clickArea.contains(sf::Vector2f(point.x, point.y) - getPosition() - parent->getPosition());
    }
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(label, states);
        target.draw(circleBorder, states);
        target.draw(circle, states);
    }
};

class RadioGroup : public sf::Transformable, public sf::Drawable {
public:
    RadioGroup(unsigned int width)
    : width(width)
    {
        topLine = sf::RectangleShape(sf::Vector2f(width, 2));
        topLine.setFillColor(dark);
        
        selectedCircle = sf::CircleShape(3);
        selectedCircle.setFillColor(dark);
        
        bottomLine = sf::RectangleShape(sf::Vector2f(width, 2));
        bottomLine.setFillColor(dark);
    }
    
    void addOption(RadioOption& option) {
        option.setPosition(0, 8 + options.size() * 34);
        options.push_back(option);
        
        if(options.size() == 1) {
            selectOption(&option);
        }
        
        bottomLine.move(0, 34 + 8);
    }
    
    void selectOption(RadioOption* option) {
        selectedOption = option;
        selectedCircle.setPosition(option->getPosition() + option->circle.getPosition() + sf::Vector2f(2, 2));
    }
    
    unsigned int getHeight() {
        return bottomLine.getPosition().y - topLine.getPosition().y + 4;
    }
    
    RadioOption* selectedOption;
    std::list<RadioOption> options;
    
    unsigned int width;
    sf::RectangleShape topLine;
    sf::CircleShape selectedCircle;
    sf::RectangleShape bottomLine;
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(topLine, states);
        
        for(RadioOption option : options) {
            target.draw(option, states);
        }
        
        target.draw(selectedCircle, states);
        target.draw(bottomLine, states);
    }
};

class Button : public sf::Transformable, public sf::Drawable {
public:
    Button(const sf::String& text, unsigned int width, const sf::Font& font, unsigned int fontSize = 26)
    : width(width)
    {
        borderRect = sf::RectangleShape(sf::Vector2f(width, 50));
        borderRect.setFillColor(dark);
        
        rect = sf::RectangleShape(sf::Vector2f(width - 4, 50 - 4));
        rect.setPosition(2, 2);
        rect.setFillColor(sf::Color::White);
        
        label = sf::Text(text, font, fontSize);
        label.setColor(dark);
        
        sf::FloatRect textRect = label.getLocalBounds();
        sf::FloatRect rectBounds = rect.getLocalBounds();
        
        label.setOrigin((int) (textRect.left + textRect.width / 2.0f),
                        (int) (textRect.top  + textRect.height / 2.0f));
        
        label.setPosition((int) (rectBounds.left + rectBounds.width / 2) + 2,
                          (int) (rectBounds.top + rectBounds.height / 2) + 2);
    }
    
    bool contains(sf::Vector2i point) {
        return rect.getGlobalBounds().contains(sf::Vector2f(point.x, point.y) - getPosition());
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

#endif
