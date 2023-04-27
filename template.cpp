#include <cstdlib>
#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;

GLdouble xVal = 0.0; // x position of the camera
GLdouble zVal = 0.0; // z position of the camera
GLdouble yVal = 40.0; // y position of the camera
GLdouble angle = 0.0; // angle of rotation for the camera direction with respect to the z axis

bool keys[256];  // an array to keep track of regular key presses
bool special_keys[256]; // an array to keep track of special key presses

bool fullscreen = false; // keeps track of whether or not the window is in fullscreen mode

// Drawing routine.
void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(xVal - 10 * sin((M_PI / 180.0) * angle), yVal, zVal - 10 * cos((M_PI / 180.0) * angle),
              xVal - 20 * sin((M_PI / 180.0) * angle), 0.0, zVal - 20 * cos((M_PI / 180.0) * angle),
              0.0, 1.0, 0.0);
    
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = -50; i <= 50; i++)
    {
        // Lines in the z axis direction
        glVertex3f(i, 0, -50);
        glVertex3f(i, 0, 50);

        // Lines in the x axis direction
        glVertex3f(-50, 0, i);
        glVertex3f(50, 0, i);
    }
    glEnd();
    
    // printf("%f", yVal);
    // fflush(stdout);
    glutSwapBuffers();
}

// This function sets up the initial state for OpenGL rendering.
void setup()
{
    // Set the clear color for the color buffer to white with 0 alpha (fully opaque)
    glClearColor(0.0, 0.0, 0.0, 0.0);

    // Enable the depth test to ensure that polygons that are behind others are not drawn
    glEnable(GL_DEPTH_TEST);

    // Enable lighting calculations for polygons
    // glEnable(GL_LIGHTING);
    
    // // Enable light source 0
    // glEnable(GL_LIGHT0);
    
    // Enable automatic normalization of surface normals to unit length
    glEnable(GL_NORMALIZE);

    // Set the polygon rasterization mode for front and back faces to solid filled mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
    gluPerspective(118, (GLfloat)w / h, 1.0, 200.0);
    // glOrtho(-100.0, 100.0, -100.0, 100.0, -100.0, 100.0);

    // Switch back to the modelview matrix.
    glMatrixMode(GL_MODELVIEW);

    // Reset the modelview matrix.
    glLoadIdentity();

    // Mark the window for redisplay.
    glutPostRedisplay();
}

// Idle function
void update()
{
    // Exit the program if the Esc key is pressed.
    if (keys[27])
        exit(0);
    
    // Increment the x and z positions. Notice how the cos is 1 when I'm moving on the z axis and the sin is 1 when I'm moving on the x axis.
    if (keys['w'])
    {
        xVal = xVal - sin(angle * M_PI / 180.0);
        zVal = zVal - cos(angle * M_PI / 180.0);
    }
    if (keys['s'])
    {
        xVal = xVal + sin(angle * M_PI / 180.0);
        zVal = zVal + cos(angle * M_PI / 180.0);
    }
    if (keys['a'])
    {
        xVal = xVal - cos(angle * M_PI / 180.0);
        zVal = zVal + sin(angle * M_PI / 180.0);
    }
    if (keys['d'])
    {
        xVal = xVal + cos(angle * M_PI / 180.0);
        zVal = zVal - sin(angle * M_PI / 180.0);
    }
    
    // Rotate the camera angle
    if (special_keys[GLUT_KEY_LEFT])
        angle = angle - 2.0;
    if (special_keys[GLUT_KEY_RIGHT])
        angle = angle + 2.0;
    // If UP is pressed then increase the y value of the camera
    if (special_keys[GLUT_KEY_UP])
        yVal = yVal + 1.0;
    // If DOWN is pressed then decrease the y value of the camera
    if (special_keys[GLUT_KEY_DOWN])
    {   
        if (yVal > 1.0)
            yVal = yVal - 1.0;
    }
    // If tab is pressed toggle full screen mode on/off
    if (keys['f'])
    {
        fullscreen = !fullscreen;
        if (fullscreen)
        {
            glutReshapeWindow(500, 500);
            glutPositionWindow(50, 50);
        }
        else
            glutFullScreen();
    }


    // Angle correction.
    if (angle > 360.0)
        angle -= 360.0;
    if (angle < 0.0)
        angle += 360.0;

    glutPostRedisplay();
}

void handleRegularKeyPress(unsigned char key, int x, int y) {
    keys[key] = true;
}

void handleRegularKeyRelease(unsigned char key, int x, int y) {
    keys[key] = false;
}

void handleSpecialKeyPress(int key, int x, int y) {
    special_keys[key] = true;
}

void handleSpecialKeyRelease(int key, int x, int y) {
    special_keys[key] = false;
}


// Main routine.
int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // Create the window
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Window");
    //glutFullScreen();
    
    // Register the callback functions.
    glutReshapeFunc(resize);
    glutKeyboardFunc(handleRegularKeyPress);
    glutKeyboardUpFunc(handleRegularKeyRelease);
    glutSpecialFunc(handleSpecialKeyPress);
    glutSpecialUpFunc(handleSpecialKeyRelease);
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
