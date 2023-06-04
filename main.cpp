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
Renderer renderer;
SoundManager sound_manager;
ISoundEngine *sound_engine = createIrrKlangDevice();

// Main routine.
int main(int argc, char **argv)
{
    
    // Initialize the framework
    glut_framework.initialize(argc, argv);

    // Initialize the inputs
    input_handler.initialize(&camera, &renderer, &sound_manager);

    // Initialize the mesh
    renderer.initialize(&camera);

    // Run the framework
    glut_framework.run();
}