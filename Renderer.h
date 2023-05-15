#ifndef RENDERER_H
#define RENDERER_H

#include <vector>

class Renderer
{
    public:
        Renderer();
        ~Renderer();
        
        void initialize(Terrain *terrain);
        static void timerCallback(int value);
        void draw();
        
    private:
        static Renderer* instance;
        GLuint vao[3];                                                      // Array of vertex array ids.
        GLuint vbo[3];                                                      // Array of buffer ids.
        std::vector<float> mesh_vertices, sun_vertices, planet_vertices;    // an array to keep track of the vertices of the terrain
        std::vector<float> mesh_colors, sun_colors, planet_colors;          // an array to keep track of the colors of the terrain
        std::vector<GLuint> mesh_indices, sun_indices, planet_indices;      // an array to keep track of the indices of the terrain

        void initializeMesh(Terrain *terrain);
        void initializeSun();
        void moveSun();
};

#endif