#include <cstdlib>
#include <cmath>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>

#include "Terrain.h"
#include "Camera.h"

#define VERTICES 0
#define INDICES 1

using namespace std;


GLint mouse_x; // keeps track of the x position of the mouse
GLint mouse_y; // keeps track of the y position of the mouse

bool keys[256];         // an array to keep track of regular key presses
bool special_keys[256]; // an array to keep track of special key presses

bool is_mouse_down = false; // keeps track of whether or not the mouse is down

bool is_polygon_filled = false; // keeps track of whether or not the polygon is filled

bool is_fullscreen = true; // keeps track of whether or not the window is in is_fullscreen mode

std::vector<float> vertices; // an array to keep track of the vertices of the terrain
std::vector<float> colors; // an array to keep track of the colors of the terrain
std::vector<GLuint> indices; // an array to keep track of the indices of the terrain

static GLuint vbo[2]; // Array of buffer ids.

Terrain *terrain;
Camera camera;

// Drawing routine.
void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(1.0, 1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    camera.update();
    
    //Increment point size
    // glPointSize(2.0);
    // glDrawArrays(GL_POINTS, 0, terrain->getDim() * terrain->getDim());
    
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
        camera.moveForward();
    if (keys['s'])
        camera.moveBackward();
    if (keys['a'])
        camera.moveLeft();
    if (keys['d'])
        camera.moveRight();
    // If Spacebar is pressed then increase the y value of the camera
    if (keys[32] && !special_keys[GLUT_KEY_SHIFT_L])
        camera.moveUp();
    // If SHIFT_L and spacebar are pressed together then decrease the y value of the camera
    if (keys[32] && special_keys[GLUT_KEY_SHIFT_L])
        camera.moveDown();
    // Rotate the camera horizontal_angle
    if (special_keys[GLUT_KEY_LEFT])
       camera.rotateLeft();
    if (special_keys[GLUT_KEY_RIGHT])
       camera.rotateRight();
    // Rotate the camera vertical_angle
    if (special_keys[GLUT_KEY_UP])
        camera.rotateUp();
    if (special_keys[GLUT_KEY_DOWN])
        camera.rotateDown();
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
        is_fullscreen = !is_fullscreen;
        if (is_fullscreen)
            glutFullScreen();
        else
        {
            glutReshapeWindow(700, 700);
            glutPositionWindow(50, 50);
        }
        keys['f'] = false;
    }
    
    if (keys['p'])
    {
        is_polygon_filled = !is_polygon_filled;
        // Set the polygon rasterization mode for front and back faces to solid filled mode
        if (is_polygon_filled)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        keys['p'] = false;
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
        is_mouse_down = true;
        mouse_x = x;
        mouse_y = y;
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
        is_mouse_down = false;

    glutPostRedisplay();
}

// Mouse motion callback routine.
void mouseMotion(int x, int y)
{
    // if (is_mouse_down)
    // {
    //     // Update the camera horizontal_angle based on the mouse movement
    //     horizontal_angle = horizontal_angle - (x - mouse_x) * 0.1;
    //     mouse_x = x;
        
    //     // Update the camera vertical_angle based on the mouse movement
    //     vertical_angle = vertical_angle - (y - mouse_y) * 0.1;
    //     mouse_y = y;
    // }
    glutPostRedisplay();
}

// Main routine.
int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    
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
