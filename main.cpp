#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Python.h>

#include "Terrain.h"
#include "Renderer.h"
#include "Camera.h"
#include "InputHandler.h"
#include "GlutFramework.h"

using namespace std;

// Global objects variables
Terrain terrain;
Camera camera;
InputHandler input_handler;
GlutFramework framework;
Renderer renderer;


// Function to execute the Python script
void predict()
{
    // Execute the Python script
    Py_Initialize();
    FILE* file = fopen("predict.py", "r");
    PyRun_SimpleFile(file, "predict.py");
    fclose(file);
    // Clean up the Python interpreter
    Py_Finalize();
}

// Main routine.
int main(int argc, char **argv)
{   
    // Initialize the framework
    framework.initialize(argc, argv);
    
    // Initialize the inputs
    input_handler.initialize(&camera, &renderer);
    
    // Allocate a terrain object
    terrain.initialize("heightmap.png", 24.0);
    
    // Initialize the mesh
    renderer.initialize(&terrain, &camera);

    // Run the framework
    framework.run();
}