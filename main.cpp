#include <cmath>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include "SOIL/SOIL.h"

#include "Terrain.h"
#include "Camera.h"
#include "InputHandler.h"
#include "GlutFramework.h"

#define VERTICES 0
#define INDICES 1

using namespace std;


std::vector<float> vertices;    // an array to keep track of the vertices of the terrain
std::vector<float> colors;      // an array to keep track of the colors of the terrain
std::vector<GLuint> indices;    // an array to keep track of the indices of the terrain

static GLuint vbo[2]; // Array of buffer ids.

// Global variables
Terrain *terrain;
Camera camera;
InputHandler input_handler;
GlutFramework framework;


// Drawing routine.
void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(1.0, 1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    camera.update();
    
    glEnable(GL_PRIMITIVE_RESTART); // Enable primitive restart
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
    glDisable(GL_PRIMITIVE_RESTART); // Disable primitive restart
    
    // Swap buffers
    glutSwapBuffers();
}

// This function sets up the initial state for OpenGL rendering.
void setup()
{
    // Set the clear color for the color buffer to white with 0 alpha (fully opaque)
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    // Enable the depth test to ensure that polygons that are behind others are not drawn
    glEnable(GL_DEPTH_TEST);
    
    // // Enable lighting calculations for polygons
    // glEnable(GL_LIGHTING);
    
    // // // Enable light source 0
    // glEnable(GL_LIGHT0);
    
    // // Enable automatic normalization of surface normals to unit length
    // glEnable(GL_NORMALIZE);
    
    // Set polygon mode to be not filled
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glGenBuffers(2, vbo);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    // Allocate a BaseTerrain object
    terrain = new Terrain("1.png", 8.0);
    
    // Get the map
    Vec3<float> *map = terrain->getMap();
    
    // Get the dimension of the map
    int dim = terrain->getDim();
    terrain->getInfo();
    
    // Allocate memory for the vertices and colors
    vertices.reserve(dim * dim * 3);
    colors.reserve(dim * dim * 3);
  
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
        vertices.emplace_back(map[i * dim + j].x());
        vertices.emplace_back(map[i * dim + j].y());
        vertices.emplace_back(map[i * dim + j].z());

        // Set the colors to be proportional to the height of the terrain and range from brown to white
        colors.emplace_back(map[i * dim + j].y() / 255.0);
        colors.emplace_back(map[i * dim + j].y() / 255.0);
        colors.emplace_back(255.0);
        }
    }
    
    // Use maximum unsigned int as restart index
    glPrimitiveRestartIndex(0xFFFFFFFFu);
    
    // Generate indices for triangle strips
    for (int z = 0; z < dim - 1; z++)
    {
        // Start a new strip
        indices.emplace_back(z * dim);
        for (int x = 0; x < dim; x++)
        {
            // Add vertices to strip
            indices.emplace_back(z * dim + x);
            indices.emplace_back((z + 1) * dim + x);
        }
        // Use primitive restart to start a new strip
        indices.emplace_back(0xFFFFFFFFu);
    }
    
    // Bind vertex buffer and reserve space.
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, (vertices.size()+colors.size())*sizeof(float), NULL, GL_STATIC_DRAW);
    
    // Copy vertex coordinates data into first half of vertex buffer.
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size()*sizeof(float), vertices.data());
    
    // Copy vertex color data into second half of vertex buffer.
    glBufferSubData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), colors.size()*sizeof(float), colors.data());
    
    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColorPointer(3, GL_FLOAT, 0, (GLvoid*)(vertices.size()*sizeof(float)));

}

// Main routine.
int main(int argc, char **argv)
{
    framework.initialize(argc, argv, &input_handler);
    input_handler.initialize(&camera);

    glutDisplayFunc(drawScene);
    
    setup();
    framework.run();
}
