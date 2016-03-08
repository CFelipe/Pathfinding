#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "ResourcePath.hpp"
#include <list>
#include <string>
#include <iostream>
#include <cmath>

const sf::Color dark = sf::Color(51, 51, 51, 255);
const sf::Color borderColor = sf::Color(102, 102, 102, 255);
const sf::Color gridBg = sf::Color(242, 242, 242, 255);
const sf::Color wallColor = sf::Color(165, 165, 165, 255);
const sf::Color startGreen = sf::Color(89, 198, 95, 255);
const sf::Color goalRed = sf::Color(196, 91, 91, 255);
const sf::Color visitedBlue = sf::Color(145, 194, 216, 255);

enum class Action { None, DraggingHandle, Painting, Erasing, DraggingRef };

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

class Node : public sf::Drawable {
public:
    Node(unsigned int width, unsigned int height, sf::Transformable* parent, const sf::Font& font)
    : wall(false),
      parent(parent),
      cameFrom(nullptr),
      heuristic(0)
    {
        rect = sf::RectangleShape(sf::Vector2f(width, height));
        rect.setFillColor(gridBg);
        heuristicLabel = sf::Text(sf::String(std::to_string(heuristic)), font, 14);
        heuristicLabel.setColor(dark);
        heuristicLabel.setPosition(5, 5);
        
    }
    
    bool getWall() {
        return wall;
    }
    
    void setWall(bool wall) {
        this->wall = wall;
        
        if(wall) {
            rect.setFillColor(wallColor);
        } else {
            rect.setFillColor(gridBg);
        }
    }
    
    sf::Vector2f center() {
        return parent->getPosition() + sf::Vector2f(rect.getPosition().x + rect.getLocalBounds().width / 2,
                                                    rect.getPosition().y + rect.getLocalBounds().height / 2);
    }
    
    bool contains(sf::Vector2i point) {
        return rect.getGlobalBounds().contains(sf::Vector2f(point.x, point.y) - parent->getPosition());
    }
    
    void link(Node* node) {
        if(!this->getWall() && !node->getWall()) {
            neighbours.push_back(node);
            node->neighbours.push_back(this);
        }
    }
    
    sf::RectangleShape rect;
    sf::Text heuristicLabel;
    std::list<Node*> neighbours;
    Node* cameFrom;
    unsigned int i;
    unsigned int j;
    unsigned int heuristic;
    
private:
    sf::Transformable* parent;
    bool wall;
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(rect, states);
        target.draw(heuristicLabel, states);
    }
};

class NodeRef : public sf::Drawable {
public:
    NodeRef() {
        node = nullptr;
    }
    
    NodeRef(Node* node, sf::Color color, sf::Transformable* parent)
    : node(node),
      parent(parent),
      dragging(false)
    {
        rect = sf::RectangleShape(sf::Vector2f(node->rect.getLocalBounds().width,
                                               node->rect.getLocalBounds().height));
        rect.setPosition(node->rect.getPosition());
        rect.setFillColor(color);
    }
    
    bool contains(sf::Vector2i point) {
        return rect.getGlobalBounds().contains(sf::Vector2f(point.x, point.y) - parent->getPosition());
    }
    
    void moveToMousePosition(sf::Vector2f mousePos) {
        if(!dragging) {
            dragging = true;
            dragOffset = rect.getPosition() - (mousePos - parent->getPosition());
        }
        
        rect.setPosition(dragOffset + (mousePos - parent->getPosition()));
    }
    
    void moveToNode() {
        rect.setPosition(node->rect.getPosition());
        dragging = false;
    }
    
    void setNode(Node* node) {
        this->node = node;
        rect.setPosition(node->rect.getPosition());
        dragging = false;
    }
    
    Node* node;
    sf::RectangleShape rect;
    
private:
    sf::Transformable* parent;
    bool dragging;
    sf::Vector2f dragOffset;
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(rect, states);
    }
};


class Grid : public sf::Transformable, public sf::Drawable {
public:
    Grid(unsigned int rows, unsigned int columns, unsigned int width, unsigned int height, const sf::Font& font)
    : rows(rows),
    columns(columns),
    width(width),
    height(height)
    {
        borderRect = sf::RectangleShape(sf::Vector2f(width + 1, height + 1));
        borderRect.setFillColor(borderColor);
        
        unsigned int nodeW = (width / columns) - 1;
        unsigned int nodeH = (height / rows) - 1;
        
        for(int j = 0; j < rows; ++j) {
            for(int i = 0; i < columns; ++i) {
                Node* node = new Node(nodeW, nodeH, this, font);
                node->rect.setPosition(1 + ((nodeW + 1) * i), 1 + ((nodeH + 1) * j));
                node->heuristicLabel.setPosition(1 + ((nodeW + 1) * i), 1 + ((nodeH + 1) * j));
                nodes[i][j] = node;
                nodes[i][j]->i = i;
                nodes[i][j]->j = j;
                
                if(i >= 1) {
                    node->link(nodes[i - 1][j]);
                }
                
                if(j >= 1) {
                    node->link(nodes[i][j - 1]);
                }
                
                if(i >= 1 && j >= 1) {
                    node->link(nodes[i - 1][j - 1]);
                }
                
                if(i + 1 < columns && j >= 1) {
                    node->link(nodes[i + 1][j - 1]);
                }
            }
        }
        
        start = NodeRef(nodes[3][3], startGreen, this);
        goal = NodeRef(nodes[8][8], goalRed, this);
        updateHeuristics();
        
        greedy();
    }
    
    bool contains(sf::Vector2i point) {
        return borderRect.getGlobalBounds().contains(sf::Vector2f(point.x, point.y) - getPosition());
    }
    
    void updateHeuristics() {
        for(int j = 0; j < rows; ++j) {
            for(int i = 0; i < columns; ++i) {
                Node* node = nodes[i][j];
                node->neighbours.clear();
                
                if(i >= 1) {
                    node->link(nodes[i - 1][j]);
                }
                
                if(j >= 1) {
                    node->link(nodes[i][j - 1]);
                }
                
                if(i >= 1 && j >= 1) {
                    node->link(nodes[i - 1][j - 1]);
                }
                
                if(i + 1 < columns && j >= 1) {
                    node->link(nodes[i + 1][j - 1]);
                }
                
                nodes[i][j]->cameFrom = nullptr;
                if(!nodes[i][j]->getWall()) {
                    nodes[i][j]->rect.setFillColor(gridBg);
                }
                // Manhattan distance
                nodes[i][j]->heuristic = abs(i - goal.node->i) + abs(j - goal.node->j);
                nodes[i][j]->heuristicLabel.setString(std::to_string(nodes[i][j]->heuristic));
            }
        }
        
        greedy();
    }
    
    void greedy() {
        std::set<Node*> openSet;
        std::set<Node*> closedSet;
        
        openSet.insert(start.node);

        while(!openSet.empty()) {
            unsigned int min = 1000;
            
            Node* current = *(openSet.begin());
            
            std::set<Node*>::iterator iterator;
            for(iterator = openSet.begin(); iterator != openSet.end(); ++iterator) {
                if((*iterator)->heuristic < min) {
                    current = *(iterator);
                    min = current->heuristic;
                }
   
            }
            
            current->rect.setFillColor(visitedBlue);
            
            if(current == goal.node) {
                return;
            }
            
            openSet.erase(current);
            closedSet.insert(current);
            
            min = 1000;
            
            for(Node* neighbour : current->neighbours) {
                if(closedSet.find(neighbour) == closedSet.end()) {
                    neighbour->cameFrom = current;
                    
                    if(neighbour->heuristic < min) {
                        min = neighbour->heuristic;
                        if(neighbour->cameFrom == nullptr) {
                            neighbour->cameFrom = current;
                        }
                    }
                    
                    openSet.insert(neighbour);
                }
            }
        }
    }
    
    Node* nodes[20][20];
    NodeRef start;
    NodeRef goal;
    
    sf::RectangleShape borderRect;
    
    unsigned int rows;
    unsigned int columns;
private:
    unsigned int width;
    unsigned int height;
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(borderRect, states);
        
        for(int i = 0; i < columns; ++i) {
            for(int j = 0; j < rows; ++j) {
                target.draw(nodes[i][j]->rect, states);
                target.draw(nodes[i][j]->heuristicLabel, states);
            }
        }
        
        target.draw(start.rect, states);
        target.draw(goal.rect, states);
    }
};

sf::RectangleShape makeLine(sf::Vector2f p1, sf::Vector2f p2, float thickness) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float rot = atan2(dy, dx) * 57.2958;
    float ht = thickness / 2;
    
    sf::RectangleShape rect(sf::Vector2f(std::sqrt(std::abs(dx)*std::abs(dx) + std::abs(dy)*std::abs(dy)), ht*2));
    rect.setSize(sf::Vector2f(std::sqrt(std::abs(dx)*std::abs(dx) + std::abs(dy)*std::abs(dy)), ht*2));
    rect.setOrigin(0, 2);
    rect.setPosition(p1.x, p1.y);
    rect.setRotation(rot);
    rect.setFillColor(dark);
    return rect;
}


int main(int, char const**) {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    
    sf::RenderWindow window(sf::VideoMode(800, 600), "Pathfinding", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile(resourcePath() + "inconsolata.otf")) {
        return EXIT_FAILURE;
    }

    Action action = Action::None;
    NodeRef* draggedRef = nullptr;
    bool mousePressed = false;
    
    // Add GUI elements ---
    unsigned int xSpace = 58;
    unsigned int ySpace = 58;

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
    
    Grid grid(15, 15, 480, 480, font);
    grid.setPosition(xSpace, ySpace);

    // ------------------------
    
    while(window.isOpen()) {
        sf::Event event;
        
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                window.close();
            }

            if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
            
            if(event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mousePosF = sf::Vector2f(mousePos.x, mousePos.y);
                
                mousePressed = true;
                
                if(grid.contains(mousePos)) {
                    if(grid.start.contains(mousePos)) {
                        action = Action::DraggingRef;
                        draggedRef = &grid.start;
                    } else if(grid.goal.contains(mousePos)) {
                        action = Action::DraggingRef;
                        draggedRef = &grid.goal;
                    } else {
                        for(int i = 0; i < grid.columns; ++i) {
                            for(int j = 0; j < grid.rows; ++j) {
                                if(grid.nodes[i][j]->contains(mousePos)) {
                                    if(!grid.nodes[i][j]->getWall()) {
                                        grid.nodes[i][j]->setWall(true);
                                        grid.updateHeuristics();
                                        action = Action::Painting;
                                    } else {
                                        grid.nodes[i][j]->setWall(false);
                                        grid.updateHeuristics();
                                        action = Action::Erasing;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            if(event.type == sf::Event::MouseMoved) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mousePosF = sf::Vector2f(mousePos.x, mousePos.y);
                
                if(action == Action::DraggingRef) {
                    draggedRef->moveToMousePosition(mousePosF);
                } else if(grid.contains(mousePos)) {
                    for(int i = 0; i < grid.columns; ++i) {
                        for(int j = 0; j < grid.rows; ++j) {
                            if(grid.nodes[i][j]->contains(mousePos)) {
                                if(action == Action::Painting) {
                                    if(grid.nodes[i][j] != grid.goal.node &&
                                       grid.nodes[i][j] != grid.start.node)
                                    {
                                        grid.updateHeuristics();
                                        grid.nodes[i][j]->setWall(true);
                                    }
                                } else if(action == Action::Erasing) {
                                    grid.updateHeuristics();
                                    grid.nodes[i][j]->setWall(false);
                                }
                            }
                        }
                    }
                }
            }
            
            if(event.type == sf::Event::MouseButtonReleased) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mousePosF = sf::Vector2f(mousePos.x, mousePos.y);
                
                mousePressed = false;
                
                if(action == Action::DraggingRef) {
                    if(grid.contains(mousePos)) {
                        for(int i = 0; i < grid.columns; ++i) {
                            for(int j = 0; j < grid.rows; ++j) {
                                if((grid.nodes[i][j]->contains(mousePos) ||
                                    grid.nodes[i][j]->contains(mousePos + sf::Vector2i(1, 1))) &&
                                   grid.nodes[i][j] != grid.start.node &&
                                   grid.nodes[i][j] != grid.goal.node)
                                {
                                    draggedRef->setNode(grid.nodes[i][j]);
                                    draggedRef->node->setWall(false);
                                    grid.updateHeuristics();
                                    grid.greedy();
                                } else {
                                    draggedRef->moveToNode();
                                }
                            }
                        }
                    } else {
                        draggedRef->moveToNode();
                    }
                }
                    
                action = Action::None;
                
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
        
        
        for(int i = 0; i < grid.columns; ++i) {
            for(int j = 0; j < grid.rows; ++j) {
                for(Node* neighbour : grid.nodes[i][j]->neighbours) {
                    if(!grid.nodes[i][j]->getWall() && !neighbour->getWall()) {
                        sf::RectangleShape rect = makeLine(grid.nodes[i][j]->center(), neighbour->center(), 2);
                        sf::Color color = rect.getFillColor();
                        color.a = 50;
                        rect.setFillColor(color);
                        window.draw(rect);
                    }
                }
            }
        }
        
        Node* node = grid.goal.node;
        
        while(node != grid.start.node && node->cameFrom != nullptr) {
            sf::RectangleShape rect = makeLine(node->center(), node->cameFrom->center(), 2);
            window.draw(rect);
            node = node->cameFrom;
        }
        
        window.display();
    }

    return EXIT_SUCCESS;
}
