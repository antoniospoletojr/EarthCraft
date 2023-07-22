#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <array>
#include <GL/glew.h>
#include "stdio.h"
#include "Vec.hpp"
#include "Constants.h"

class QuadTree
{
private:
    struct Node
    {
        // The bounds of the node's square
        float min_x, max_x, min_z, max_z;
        
        // Indices contained within the node
        std::vector<GLuint> indices;

        // Child nodes
        std::array<Node *, 4> children;
        
        Node(float min_x, float max_x, float min_z, float max_z);

        ~Node();
    };
    
    std::vector<GLuint> visible_indices;
    Node *root;

public:
    static const GLuint MaxIndicesPerNode = 8;

    QuadTree(float min_x, float max_x, float min_z, float max_z);

    ~QuadTree();

    void build(const std::vector<GLuint> &indices);
    
    void clear();
    
    void insert(GLuint index, Node *node);
    
    void subdivide(Node *node);
    
    bool isIndexInBounds(GLuint index, Node *node) const;
    
    std::vector<GLuint> *frustumCull(Vec3<float> position, Vec3<float> direction);

};

#endif // QUADTREE_H
