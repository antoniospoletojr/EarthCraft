#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <array>
#include <cmath>
#include <GL/glew.h>
#include "stdio.h"
#include "Vec.hpp"
#include "Constants.h"
#include "Object.h"
#include "Terrain.h"

// Forward declaration
class QuadTree;
class QuadNode
{
    private:
        // The bounds of the node's square (indexes of the terrain's vertices)
        float x, z, width, depth;
        
        // Node boundaries in world coordinates
        Vec2<float> top_left_corner;
        Vec2<float> top_right_corner;
        Vec2<float> bottom_left_corner;
        Vec2<float> bottom_right_corner;
        
        // Child nodes
        QuadNode *NW_child;
        QuadNode *NE_child;
        QuadNode *SW_child;
        QuadNode *SE_child;
        
        // Objects contained in the node
        Object object;
        
        // Reference to the parent class
        QuadTree *quadtree;
    
        QuadNode(QuadTree *quadtree, Terrain *terrain, float x, float z, float width, float depth);
        
        ~QuadNode();
        
        void draw();

    friend class QuadTree;
};

class QuadTree
{
private:
    QuadNode *root;
    float fov_angle;
    Vec2<float> camera_position, camera_direction;
    GLuint texture_id;
    

public:
    
    QuadTree();
    
    ~QuadTree();
    
    void initialize(Terrain *terrain);
    
    void render(Vec2<float> position, Vec2<float> direction);
    
    bool isInFrustum(QuadNode *node);
    
    // void clear();
    
    // void insert(GLuint index, Node *node);
    
    // void subdivide(Node *node);
    
    // bool isIndexInBounds(GLuint index, Node *node) const;
    
    // std::vector<GLuint> *frustumCull(Vec3<float> position, Vec3<float> direction);

};

#endif // QUADTREE_H
