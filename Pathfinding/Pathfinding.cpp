#include "Pathfinding.h"
#include <iostream>

const sf::Color dark = sf::Color(51, 51, 51, 255);
const sf::Color borderColor = sf::Color(102, 102, 102, 255);
const sf::Color gridBg = sf::Color(242, 242, 242, 255);
const sf::Color wallColor = sf::Color(140, 140, 140, 255);
const sf::Color startGreen = sf::Color(89, 198, 95, 255);
const sf::Color goalRed = sf::Color(196, 91, 91, 255);
const sf::Color visitedBlue = sf::Color(145, 194, 216, 255);
const sf::Color openBlue = sf::Color(77, 209, 255, 255);

Node::Node(unsigned int width, unsigned int height, sf::Transformable* parent, const sf::Font& font)
    : wall(false),
    parent(parent),
    cameFrom(nullptr),
    heuristic(0),
    gCost(1000),
    fCost(1000)
{
    rect = sf::RectangleShape(sf::Vector2f(width, height));
    rect.setFillColor(gridBg);
    
    heuristicLabel = sf::Text("", font, 9);
    heuristicLabel.setColor(dark);
    
    gCostLabel = sf::Text("", font, 9);
    gCostLabel.setColor(dark);
    
    fCostLabel = sf::Text("", font, 9);
    fCostLabel.setColor(dark);
}

bool Node::getWall() {
    return wall;
}
    
void Node::setWall(bool wall) {
    this->wall = wall;
    
    if(wall) {
        rect.setFillColor(wallColor);
    } else {
        rect.setFillColor(gridBg);
    }
}
    
sf::Vector2f Node::center() {
    return parent->getPosition() +
            sf::Vector2f(rect.getPosition().x + rect.getLocalBounds().width / 2,
                         rect.getPosition().y + rect.getLocalBounds().height / 2);
}
    
bool Node::contains(sf::Vector2i point) {
    return rect.getGlobalBounds().contains(sf::Vector2f(point.x, point.y) - parent->getPosition());
}
    
void Node::link(Node* node) {
    if(!this->getWall() && !node->getWall()) {
        neighbours.push_back(node);
        node->neighbours.push_back(this);
    }
}

void Node::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(rect, states);
    target.draw(heuristicLabel, states);
    target.draw(gCostLabel, states);
    target.draw(fCostLabel, states);
}

void PFAlgorithm::iterate() {
    ++iteration;
    
    grid.updateHeuristics();
    reset();
    run();
}
    
void PFAlgorithm::rewind() {
    if(iteration >= 1)
        iteration -= 1;
    
    grid.updateHeuristics();
    reset();
    run();
}

void PFAlgorithm::reset() {
    openSet.clear();
    closedSet.clear();
    
    iterations = 0;
}

void PFAlgorithm::toStart() {
    iteration = 0;
    
    grid.updateHeuristics();
    reset();
    run();
}

void PFAlgorithm::toEnd() {
    grid.updateHeuristics();
    reset();
    run(true);
}

NodeRef::NodeRef(Node* node, sf::Color color, sf::Transformable* parent)
: node(node),
  parent(parent),
  dragging(false)
{
    rect = sf::RectangleShape(sf::Vector2f(node->rect.getLocalBounds().width,
                                           node->rect.getLocalBounds().height));
    rect.setPosition(node->rect.getPosition());
    rect.setFillColor(color);
}
    
bool NodeRef::contains(sf::Vector2i point) {
    return rect.getGlobalBounds().contains(sf::Vector2f(point.x, point.y) - parent->getPosition());
}
    
void NodeRef::moveToMousePosition(sf::Vector2f mousePos) {
    if(!dragging) {
        dragging = true;
        dragOffset = rect.getPosition() - (mousePos - parent->getPosition());
    }
    
    rect.setPosition(dragOffset + (mousePos - parent->getPosition()));
}
    
void NodeRef::moveToNode() {
    rect.setPosition(node->rect.getPosition());
    dragging = false;
}
    
void NodeRef::setNode(Node* node) {
    this->node = node;
    rect.setPosition(node->rect.getPosition());
    dragging = false;
}
    
void NodeRef::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(rect, states);
}


Grid::Grid(unsigned int rows, unsigned int columns, unsigned int width, unsigned int height, const sf::Font& font)
: rows(rows),
  columns(columns),
  width(width),
  height(height),
  algorithm(nullptr)
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
            node->gCostLabel.setPosition(1 + ((nodeW + 1) * i), 11 + ((nodeH + 1) * j));
            node->fCostLabel.setPosition(1 + ((nodeW + 1) * i), 21 + ((nodeH + 1) * j));
            nodes[i][j] = node;
            nodes[i][j]->i = i;
            nodes[i][j]->j = j;
        }
    }
    
    start = NodeRef(nodes[3][3], startGreen, this);
    goal = NodeRef(nodes[8][8], goalRed, this);
    updateHeuristics();
}
    
bool Grid::contains(sf::Vector2i point) {
    return borderRect.getGlobalBounds().contains(sf::Vector2f(point.x, point.y) - getPosition());
}

void Grid::clearWalls() {
    for(int j = 0; j < rows; ++j) {
        for(int i = 0; i < columns; ++i) {
            nodes[i][j]->setWall(false);
            updateHeuristics();
        }
    }
}

void Grid::updateHeuristics() {
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
            
            // Diagonals
            
            if(i >= 1 && j >= 1) {
                node->link(nodes[i - 1][j - 1]);
            }
            
            if(i + 1 < columns && j >= 1) {
                node->link(nodes[i + 1][j - 1]);
            }
            
            
            nodes[i][j]->fCost = 1000;
            nodes[i][j]->gCost = 1000;
            
            nodes[i][j]->cameFrom = nullptr;
            if(!nodes[i][j]->getWall()) {
                nodes[i][j]->rect.setFillColor(gridBg);
            }
            // Manhattan distance
            nodes[i][j]->heuristic = abs(i - goal.node->i) + abs(j - goal.node->j);
            nodes[i][j]->heuristicLabel.setString(std::to_string(nodes[i][j]->heuristic));
            nodes[i][j]->gCostLabel.setString("");
            nodes[i][j]->fCostLabel.setString("");
        }
    }
    
    start.node->gCost = 0;
    
    if(algorithm != nullptr) {
        algorithm->reset();
        algorithm->run();
    }
}

unsigned int Grid::movCost(Node* n1, Node* n2) {
    return abs(n1->i - n2->i) + abs(n1->j - n2->j) > 1 ? 14 : 10;
}

bool Grid::getWall(unsigned int i, unsigned int j) {
    return nodes[i][j]->getWall();
}

void Grid::setWall(unsigned int i, unsigned int j, bool wall) {
    nodes[i][j]->setWall(wall);
    updateHeuristics();
}

void Grid::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(borderRect, states);
    
    for(int i = 0; i < columns; ++i) {
        for(int j = 0; j < rows; ++j) {
            target.draw(*nodes[i][j], states);
        }
    }
    
    target.draw(start.rect, states);
    target.draw(goal.rect, states);
}
    
void AStar::run(bool toEnd) {
    openSet.insert(grid.start.node);
    
    while(!openSet.empty() && (iterations < iteration || toEnd)) {
        unsigned int min = 1000;
        
        Node* current = *(openSet.begin());
        
        std::set<Node*>::iterator iterator;
        for(iterator = openSet.begin(); iterator != openSet.end(); ++iterator) {
            if((*iterator)->fCost < min) {
                current = *(iterator);
                min = current->fCost;
            }
        }
        
        current->rect.setFillColor(openBlue);
        
        if(current == grid.goal.node) {
            break;
        }
        
        openSet.erase(current);
        closedSet.insert(current);
        
        unsigned int newCost = 1000;
        
        for(Node* neighbour : current->neighbours) {
            if(closedSet.find(neighbour) == closedSet.end()) {
                neighbour->rect.setFillColor(visitedBlue);
                
                openSet.insert(neighbour);
                newCost = current->gCost + grid.movCost(current, neighbour);
                
                //neighbour->fCostLabel.setString(std::to_string(newCost));
                if(newCost < neighbour->gCost) {
                    neighbour->gCost = newCost;
                    neighbour->gCostLabel.setString(std::to_string(neighbour->gCost));
                    
                    neighbour->fCost = newCost + neighbour->heuristic;
                    neighbour->fCostLabel.setString(std::to_string(neighbour->fCost));
                    neighbour->cameFrom = current;
                }
                
            }
        }
        
        ++iterations;
    }
    
    if(toEnd) {
        iteration = iterations;
    }
}

void Greedy::run(bool toEnd) {
    openSet.insert(grid.start.node);
    
    while(!openSet.empty() && (iterations < iteration || toEnd)) {
        unsigned int min = 1000;
        
        Node* current = *(openSet.begin());
        
        std::set<Node*>::iterator iterator;
        for(iterator = openSet.begin(); iterator != openSet.end(); ++iterator) {
            if((*iterator)->heuristic < min) {
                current = *(iterator);
                min = current->heuristic;
            }
            
        }
        
        current->rect.setFillColor(openBlue);
        
        if(current == grid.goal.node) {
            break;
        }
        
        openSet.erase(current);
        closedSet.insert(current);
        
        min = 1000;
        
        for(Node* neighbour : current->neighbours) {
            if(closedSet.find(neighbour) == closedSet.end()) {
                neighbour->rect.setFillColor(visitedBlue);

                if(neighbour->heuristic < min && neighbour->cameFrom == nullptr) {
                    neighbour->cameFrom = current;
                }
                
                openSet.insert(neighbour);
            }
        }
        
        ++iterations;
    }
    
    if(toEnd) {
        iteration = iterations;
    }
}
