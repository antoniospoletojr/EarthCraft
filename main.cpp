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


// Drawing routine.
void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    camera.update();
    renderer.draw();
    
    glutSwapBuffers();
}

// This function sets up the initial state for OpenGL rendering.
void setup()
{
    // Set the clear color for the color buffer to white with 0 alpha (fully opaque)
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    // Set polygon mode to be not filled
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // Enable the depth test to ensure that polygons that are behind others are not drawn
    glEnable(GL_DEPTH_TEST);
    
    // // Enable lighting calculations for polygons
    // glEnable(GL_LIGHTING);
    
    // // Enable light source 0
    // glEnable(GL_LIGHT0);
    
    // // Enable automatic normalization of surface normals to unit length
    // glEnable(GL_NORMALIZE);
    
    // Allocate a terrain object
    terrain.initialize("./data/heightmap.png", 8.0);
    
    // Initialize the mesh
    renderer.initialize(&terrain);
}

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
    // Initialize the framework and input handler
    framework.initialize(argc, argv);
    input_handler.initialize(&camera);
    
    glutDisplayFunc(drawScene);
    
    setup();
    framework.run();
}