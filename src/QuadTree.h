/**
@file
@brief QuadTree header file.
*/

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

/**
 * @brief QuadTree node class for frustrum culling.
 * 
 * This class represents a node of the QuadTree. It contains the node's boundaries, the node's object and the node's children.
 * Each node represents a square portion of the terrain's map.
 */
class QuadNode
{
    private:
        // The bounds of the node's square (indexes of the terrain's vertices)
        float x;        ///< Column-coordinate of the node's top left corner as an index of the terrain's vertices
        float z;        ///< Row-coordinate of the node's top left corner as an index of the terrain's vertices
        float width;    ///< Width of the node's square as an index of the terrain's vertices
        float depth;    ///< Depth of the node's square as an index of the terrain's vertices
        
        // Node boundaries in world coordinates
        Vec2<float> top_left_corner;        ///< Top left corner of the node in world coordinates
        Vec2<float> top_right_corner;       ///< Top right corner of the node in world coordinates
        Vec2<float> bottom_left_corner;     ///< Bottom left corner of the node in world coordinates
        Vec2<float> bottom_right_corner;    ///< Bottom right corner of the node in world coordinates
        
        // Child nodes
        QuadNode *NW_child; ///< North-West child node
        QuadNode *NE_child; ///< North-East child node
        QuadNode *SW_child; ///< South-West child node
        QuadNode *SE_child; ///< South-East child node
        
        Object object; ///< Object contained in the node, containing all the opengl buffers of the node
        
        QuadTree *quadtree; ///< Reference to the parent class

        /**
         * @brief Construct a new Quad Node object
         * 
         * @param quadtree Reference to the parent class
         * @param terrain Reference to the terrain object, useful for accessing the respective sub portion of the map
         * @param x Top left corner column-coordinate of the node's square as an index of the terrain's vertices
         * @param z Top left corner row-coordinate of the node's square as an index of the terrain's vertices
         * @param width Width of the node's square as an index of the terrain's vertices
         * @param depth Depth of the node's square as an index of the terrain's vertices
         */
        QuadNode(QuadTree *quadtree, Terrain *terrain, float x, float z, float width, float depth);
        
        /**
         * @brief Destroy the Quad Node object
         * 
         */
        ~QuadNode();
        
        /**
         * @brief Recursively subdivide the node into 4 children nodes when needed (when the node is in the frustum) and draw the node's object
         * 
         */
        void draw();

    friend class QuadTree;
};

/**
 * @brief QuadTree class for frustrum culling. 
 * 
 * This class represents the QuadTree. It contains the root node of the tree and the camera's position and direction updated
 * on each frame, together with the texture id of the terrain's texture covering all of his children nodes.
 */
class QuadTree
{
private:
    QuadNode *root;                 ///< Root node of the QuadTree
    float fov_angle;                ///< Field of view angle of the frustrum in degrees
    Vec2<float> camera_position;    ///< Position of the camera in world coordinates
    Vec2<float> camera_direction;   ///< Direction of the camera in world coordinates
    GLuint texture_id;              ///< Texture id of the terrain's texture
    

public:
    
    /**
     * @brief Construct a new Quad Tree object
     * 
     */
    QuadTree();
    
    /**
     * @brief Destroy the Quad Tree object
     * 
     */
    ~QuadTree();
    
    /**
     * @brief Initialize the QuadTree by creating the root node and binding the entire terrain texture
     * 
     * @param terrain Reference to the terrain object
     */
    void initialize(Terrain *terrain);
    
    /**
     * @brief Update the camera's position and direction and render the QuadTree by calling the draw() function of the root node
     * 
     * @param camera_position Position of the camera in world coordinates
     * @param camera_direction Direction of the camera in world coordinates
     */
    void render(Vec2<float> position, Vec2<float> direction);
    
    /**
     * @brief Check if a node is in the frustum
     * 
     *  The function checks the angle between the camera direction vector and node position vector (each of its 4 corners) and
     *  returns true if the angle is less than the field of view angle of the frustrum. 
     * 
     * @param node Node to check
     * @return true If the node is in the frustum
     * @return false If the node is not in the frustum
     */
    bool isInFrustum(QuadNode *node);

};

#endif // QUADTREE_H
