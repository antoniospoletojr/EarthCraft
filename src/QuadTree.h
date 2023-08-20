#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <array>
#include <GL/glew.h>
#include "stdio.h"
#include "Vec.hpp"
#include "Constants.h"
#include "Object.h"
#include "Terrain.h"


class QuadNode
{
    // The bounds of the node's square
    float x, z, width, depth;
    
    // Child nodes
    QuadNode *NW_child;
    QuadNode *NE_child;
    QuadNode *SW_child;
    QuadNode *SE_child;
    
    // Objects contained in the node
    Object object;
    
    QuadNode(Terrain *terrain, float x, float z, float width, float depth);
    
    ~QuadNode();

    void draw();

    friend class QuadTree;
};

class QuadTree
{
private:
    QuadNode *root;
    

public:
    
    QuadTree();

    ~QuadTree();

    void initialize(Terrain *terrain);

    void draw();
    
    // void clear();
    
    // void insert(GLuint index, Node *node);
    
    // void subdivide(Node *node);
    
    // bool isIndexInBounds(GLuint index, Node *node) const;
    
    // std::vector<GLuint> *frustumCull(Vec3<float> position, Vec3<float> direction);

};

#endif // QUADTREE_H
