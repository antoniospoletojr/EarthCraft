#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Terrain.h"
#include "Renderer.h"
#include "Camera.h"
#include "InputHandler.h"
#include "GlutFramework.h"

using namespace std;

// Global objects variables
Camera camera;
InputHandler input_handler;
GlutFramework framework;
Renderer renderer;


// Main routine.
int main(int argc, char **argv)
{   
    // Initialize the framework
    framework.initialize(argc, argv);
    
    // Initialize the inputs
    input_handler.initialize(&camera, &renderer);
    
    // Initialize the mesh
    renderer.initialize(&camera);

    // Run the framework
    framework.run();
}