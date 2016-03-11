#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "ResourcePath.hpp"
#include <list>
#include <string>
#include <iostream>
#include <cmath>
#include "GUI.h"
#include "Pathfinding.h"

enum class Action { None, DraggingHandle, Painting, Erasing, DraggingRef };

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
    RadioOption aStarOption(sf::String(L"A*"), font, &radioGroup);
    radioGroup.addOption(aStarOption);
    RadioOption greedyOption(sf::String(L"Gulosa"), font, &radioGroup);
    radioGroup.addOption(greedyOption);
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
    
    AStar aStar(grid);
    Greedy greedy(grid);
    grid.algorithm = &aStar;
    grid.updateHeuristics();
    
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
                                        grid.setWall(i,j, true);
                                        action = Action::Painting;
                                    } else {
                                        grid.setWall(i, j, false);
                                        action = Action::Erasing;
                                    }
                                }
                            }
                        }
                    }
                }
                
                if(cleanButton.contains(mousePos)) {
                    grid.clearWalls();
                }
                
                if(iterateButton.contains(mousePos)) {
                    grid.algorithm->iterate();
                }

                if(rewindButton.contains(mousePos)) {
                    grid.algorithm->rewind();
                }
                
                if(endButton.contains(mousePos)) {
                    grid.algorithm->toEnd();
                }
                
                if(aStarOption.contains(mousePos)) {
                    radioGroup.selectOption(&aStarOption);
                    grid.algorithm = &aStar;
                    grid.updateHeuristics();
                }
                
                if(greedyOption.contains(mousePos)) {
                    radioGroup.selectOption(&greedyOption);
                    grid.algorithm = &greedy;
                    grid.updateHeuristics();
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
                                        grid.setWall(i, j, true);
                                    }
                                } else if(action == Action::Erasing) {
                                    grid.setWall(i, j, false);
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
                                    grid.setWall(i, j, false);
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
        
        /*
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
         */
        
        if(grid.goal.node->cameFrom == nullptr) {
            for(int i = 0; i < grid.columns; ++i) {
                for(int j = 0; j < grid.rows; ++j) {
                    for(Node* neighbour : grid.nodes[i][j]->neighbours) {
                        if(!grid.nodes[i][j]->getWall() && !neighbour->getWall()) {
                            if(grid.nodes[i][j]->cameFrom != nullptr) {
                                sf::RectangleShape rect = makeLine(grid.nodes[i][j]->center(), grid.nodes[i][j]->cameFrom->center(), 2);
                                sf::Color color = rect.getFillColor();
                                color.a = 50;
                                rect.setFillColor(color);
                                window.draw(rect);
                            }
                        }
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
