#include <cstdlib>
#include <cmath>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "terrain.h"

#define VERTICES 0
#define INDICES 1

using namespace std;

GLdouble x_val = 0.0;   // x position of the camera
GLdouble z_val = 900.0; // z position of the camera
GLdouble y_val = 30.0;  // y position of the camera

GLdouble horizontal_angle = 0.0;  // horizontal_angle of rotation for the camera direction on the xz plane with respect to the z axis
GLdouble vertical_angle = 0.0;    // vertical_angle of rotation for the camera direction on the yz plane with respect to the y axis

GLint mouse_x; // keeps track of the x position of the mouse
GLint mouse_y; // keeps track of the y position of the mouse

bool keys[256];         // an array to keep track of regular key presses
bool special_keys[256]; // an array to keep track of special key presses

bool mouse_down = false; // keeps track of whether or not the mouse is down

bool fullscreen = true; // keeps track of whether or not the window is in fullscreen mode

static float *vertices; // an array to keep track of the vertices of the terrain
static float *colors;   // an array to keep track of the colors of the terrain

static unsigned int buffer[1]; // Array of buffer ids.

Terrain *terrain;

// Drawing routine.
void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(1.0, 1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(x_val - 1 * sin((M_PI / 180.0) * horizontal_angle), y_val , z_val - 1 * cos((M_PI / 180.0) * horizontal_angle),
              x_val - 2 * sin((M_PI / 180.0) * horizontal_angle), y_val + 0.01 * vertical_angle, z_val - 2 * cos((M_PI / 180.0) * horizontal_angle),
              0.0, 1.0, 0.0);
    
    //Increment point size
    glPointSize(2.0);
    
    // Draw the terrain
    glDrawArrays(GL_POINTS, 0, terrain->getDim() * terrain->getDim());
    
    // Swap buffers
    glutSwapBuffers();
}

// This function sets up the initial state for OpenGL rendering.
void setup()
{
    // Set the clear color for the color buffer to white with 0 alpha (fully opaque)
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    // Enable the depth test to ensure that polygons that are behind others are not drawn
    // glEnable(GL_DEPTH_TEST);
    
    // // Enable lighting calculations for polygons
    // glEnable(GL_LIGHTING);worldScale
    
    // // Enable light source 0
    // glEnable(GL_LIGHT0);
    
    // Enable automatic normalization of surface normals to unit length
    glEnable(GL_NORMALIZE);
    
    // Set the polygon rasterization mode for front and back faces to solid filled mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glGenBuffers(1, buffer);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    // Allocate a BaseTerrain object
    terrain = new Terrain("1.png", 8.0);
    
    // Get the map
    Vertex *map = terrain->getMap();
    
    // Get the dimension of the map
    int dim = terrain->getDim();
    terrain->getInfo();
    
    // Allocate memory for the vertices and colors
    vertices = new float[dim * dim * 3];
    colors = new float[dim * dim * 3];
    
    // Set the vertices and colors
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            vertices[(i * dim + j) * 3] = map[i * dim + j].x;
            vertices[(i * dim + j) * 3 + 1] = map[i * dim + j].y;
            vertices[(i * dim + j) * 3 + 2] = map[i * dim + j].z;
            
            colors[(i * dim + j) * 3] = 1.0;
            colors[(i * dim + j) * 3 + 1] = 1.0;
            colors[(i * dim + j) * 3 + 2] = 1.0;
        }
    }
    
    // Bind vertex buffer and reserve space.
    glBindBuffer(GL_ARRAY_BUFFER, buffer[VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, dim*dim*3*sizeof(float)*2, NULL, GL_STATIC_DRAW);
    
    // Copy vertex coordinates data into first half of vertex buffer.
    glBufferSubData(GL_ARRAY_BUFFER, 0, dim*dim*3*sizeof(float), vertices);
    
    // Copy vertex color data into second half of vertex buffer.
    glBufferSubData(GL_ARRAY_BUFFER, dim*dim*3*sizeof(float), dim*dim*3*sizeof(float), colors);

    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColorPointer(3, GL_FLOAT, 0, (GLvoid*)(dim*dim*3*sizeof(float)));
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    // Set up the viewport to cover the entire window.
    glViewport(0, 0, w, h);
    
    // Switch to the projection matrix.
    glMatrixMode(GL_PROJECTION);
    
    // Reset the projection matrix.
    glLoadIdentity();
    
    // Set up a perspective projection with a field of view of 118 degrees, an aspect ratio of w/h, and a near/far clipping plane of 30.0 and 100.0 respectively.
    gluPerspective(50, (GLfloat)w / h, 1.0, 3000.0);
    // glOrtho(-100.0, 100.0, -100.0, 100.0, -100.0, 100.0);
    
    // Switch back to the modelview matrix.
    glMatrixMode(GL_MODELVIEW);
    
    // Reset the modelview matrix.
    glLoadIdentity();
    
    // Mark the window for redisplay.
    glutPostRedisplay();
}

void handleCamera()
{
    // Increment the x and z positions. Notice how the cos is 1 when I'm moving on the z axis and the sin is 1 when I'm moving on the x axis.
    if (keys['w'])
    {
        x_val = x_val - sin(horizontal_angle * M_PI / 180.0);
        z_val = z_val - cos(horizontal_angle * M_PI / 180.0);
    }
    if (keys['s'])
    {
        x_val = x_val + sin(horizontal_angle * M_PI / 180.0);
        z_val = z_val + cos(horizontal_angle * M_PI / 180.0);
    }
    if (keys['a'])
    {
        x_val = x_val - cos(horizontal_angle * M_PI / 180.0);
        z_val = z_val + sin(horizontal_angle * M_PI / 180.0);
    }
    if (keys['d'])
    {
        x_val = x_val + cos(horizontal_angle * M_PI / 180.0);
        z_val = z_val - sin(horizontal_angle * M_PI / 180.0);
    }
    
    // Rotate the camera horizontal_angle
    if (special_keys[GLUT_KEY_LEFT])
        horizontal_angle = horizontal_angle + 1.0;
    if (special_keys[GLUT_KEY_RIGHT])
        horizontal_angle = horizontal_angle - 1.0;
    // Rotate the camera vertical_angle
    if (special_keys[GLUT_KEY_UP])
        if (vertical_angle < 180.0)
            vertical_angle = vertical_angle + 1.0;
    if (special_keys[GLUT_KEY_DOWN])
        if (vertical_angle > -180.0)
            vertical_angle = vertical_angle - 1.0;

    // If Spacebar is pressed then increase the y value of the camera
    if (keys[32] && !special_keys[GLUT_KEY_SHIFT_L])
        y_val = y_val + 1.0;
    // If SHIFT_L and spacebar are pressed together then decrease the y value of the camera
    if (keys[32] && special_keys[GLUT_KEY_SHIFT_L])
        if (y_val > 1.0)
            y_val = y_val - 1.0;

    // horizontal_angle correction.
    if (horizontal_angle > 360.0)
        horizontal_angle -= 360.0;
    if (horizontal_angle < 0.0)
        horizontal_angle += 360.0;
}

// Idle function
void update()
{
    // Exit the program if the Esc key is pressed.
    if (keys[27] || keys['q'])
        exit(0);

    // If tab is pressed toggle full screen mode on/off
    if (keys['f'])
    {
        fullscreen = !fullscreen;
        if (fullscreen)
            glutFullScreen();
        else
        {
            glutReshapeWindow(700, 700);
            glutPositionWindow(50, 50);
        }
        
        keys['f'] = false;
    }

    handleCamera();
    
    glutPostRedisplay();
}

void handleRegularKeyPress(unsigned char key, int x, int y)
{
    keys[key] = true;
}

void handleRegularKeyRelease(unsigned char key, int x, int y)
{
    keys[key] = false;
}

void handleSpecialKeyPress(int key, int x, int y)
{
    special_keys[key] = true;
}

void handleSpecialKeyRelease(int key, int x, int y)
{
    special_keys[key] = false;
}

void mouseClick(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        mouse_down = true;
        mouse_x = x;
        mouse_y = y;
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
        mouse_down = false;

    glutPostRedisplay();
}

// Mouse motion callback routine.
void mouseMotion(int x, int y)
{
    if (mouse_down)
    {
        // Update the camera horizontal_angle based on the mouse movement
        horizontal_angle = horizontal_angle - (x - mouse_x) * 0.1;
        mouse_x = x;

        // Update the camera vertical_angle based on the mouse movement
        vertical_angle = vertical_angle - (y - mouse_y) * 0.1;
        mouse_y = y;
    }
    glutPostRedisplay();
}

// Main routine.
int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // Create the window
    // glutInitWindowSize(700, 700);
    // glutInitWindowPosition(100, 100);
    glutCreateWindow("Window");
    glutFullScreen();

    // Register the callback functions.
    glutReshapeFunc(resize);

    glutKeyboardFunc(handleRegularKeyPress);
    glutKeyboardUpFunc(handleRegularKeyRelease);
    glutSpecialFunc(handleSpecialKeyPress);
    glutSpecialUpFunc(handleSpecialKeyRelease);

    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMotion);

    glutDisplayFunc(drawScene);
    glutIdleFunc(update);

    // Initialize GLEW.
    glewExperimental = GL_TRUE;
    glewInit();

    // Set up the scene.
    setup();

    // Enter the main loop.
    glutMainLoop();
}
