#ifndef __Pathfinding__Pathfinding__
#define __Pathfinding__Pathfinding__

#include <SFML/Graphics.hpp>
#include <list>
#include <set>

class Node : public sf::Drawable {
public:
    Node(unsigned int width, unsigned int height, sf::Transformable* parent, const sf::Font& font);
    
    bool getWall();
    void setWall(bool wall);
    sf::Vector2f center();
    bool contains(sf::Vector2i point);
    void link(Node* node);
    
    sf::RectangleShape rect;
    sf::Text heuristicLabel;
    sf::Text gCostLabel;
    sf::Text fCostLabel;
    std::list<Node*> neighbours;
    Node* cameFrom;
    unsigned int i;
    unsigned int j;
    unsigned int heuristic;
    unsigned int gCost;
    unsigned int fCost;
private:
    sf::Transformable* parent;
    bool wall;
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class NodeRef : public sf::Drawable {
public:
    NodeRef() : node(nullptr) {};
    NodeRef(Node* node, sf::Color color, sf::Transformable* parent);
    
    bool contains(sf::Vector2i point);
    void moveToMousePosition(sf::Vector2f mousePos);
    void moveToNode();
    void setNode(Node* node);
    
    Node* node;
    sf::RectangleShape rect;

private:
    sf::Transformable* parent;
    bool dragging;
    sf::Vector2f dragOffset;
    
    virtual void  draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class Grid;

class PFAlgorithm {
public:
    PFAlgorithm(Grid& grid)
    : grid(grid),
    iteration(0),
    iterations(0)
    {}
    
    void iterate();
    void rewind();
    void reset();
    void toEnd();
    virtual void run(bool toEnd = false) = 0;
    
    std::set<Node*> openSet;
    std::set<Node*> closedSet;
    Grid& grid;
    unsigned int iterations;
    unsigned int iteration;
};

class Grid : public sf::Transformable, public sf::Drawable {
public:
    Grid(unsigned int rows, unsigned int columns, unsigned int width, unsigned int height, const sf::Font& font);
    
    Node* nodes[20][20];
    NodeRef start;
    NodeRef goal;
    PFAlgorithm* algorithm;
    
    unsigned int rows;
    unsigned int columns;
    unsigned int width;
    unsigned int height;
    
    sf::RectangleShape borderRect;
    
    unsigned int movCost(Node* n1, Node* n2);
    bool getWall(unsigned int i, unsigned int j);
    void setWall(unsigned int i, unsigned int j, bool wall);
    void movCost();
    bool contains(sf::Vector2i point);
    void clearWalls();
    void updateHeuristics();
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class AStar : public PFAlgorithm {
public:
    AStar(Grid& grid) : PFAlgorithm(grid) {}
    
    virtual void run(bool toEnd);
};

class Greedy : public PFAlgorithm {
public:
    Greedy(Grid& grid) : PFAlgorithm(grid) {}
    
    virtual void run(bool toEnd);
};

#endif /* defined(__Pathfinding__Pathfinding__) */
