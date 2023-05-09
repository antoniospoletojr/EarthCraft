#ifndef MESH_H
#define MESH_H

#include <vector>

class Mesh
{
    public:
        Mesh();
        ~Mesh();
        
        void initialize(Terrain *terrain);
        void draw();
        
    private:
        GLuint vbo[2];                  // Array of buffer ids.
        std::vector<float> vertices;    // an array to keep track of the vertices of the terrain
        std::vector<float> colors;      // an array to keep track of the colors of the terrain
        std::vector<GLuint> indices;    // an array to keep track of the indices of the terrain
};

#endif