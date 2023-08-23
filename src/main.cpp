/**
@file
@brief Center source of the program. Initializes all main objects and runs the framework.
*/

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Terrain.h"
#include "Renderer.h"
#include "Camera.h"
#include "InputHandler.h"
#include "GlutFramework.h"
#include "SoundManager.h"

using namespace std;

// Global objects variables
Camera camera;
InputHandler input_handler;
GlutFramework glut_framework;
SoundManager sound_manager;
Renderer renderer;
QuadTree quadtree;

int main(int argc, char **argv)
{
    // Initialize the framework
    glut_framework.initialize(argc, argv);
    
    // Initialize the inputs
    input_handler.initialize(&camera, &renderer, &sound_manager, &quadtree);
    
    // Initialize the mesh
    renderer.initialize(&camera, &quadtree);
    
    // Run the framework
    glut_framework.run();
}