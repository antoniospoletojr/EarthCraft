#include <GL/glew.h>
#include <GL/freeglut.h>
#include "GlutFramework.h"

GlutFramework* GlutFramework::instance = nullptr;

// Default constructor
GlutFramework::GlutFramework()
{
    if (GlutFramework::instance == nullptr)
        GlutFramework::instance = this;
}

// Destructor
GlutFramework::~GlutFramework()
{
    GlutFramework::instance = nullptr;
}

void GlutFramework::initialize(int argc, char** argv)
{    
    glutInit(&argc, argv);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("Window");
    glutFullScreen();
    glutReshapeFunc(GlutFramework::resize);
    
    glewExperimental = GL_TRUE;
    glewInit();
}

// OpenGL window reshape routine.
void GlutFramework::resize(int w, int h)
{
    // Set up the viewport to cover the entire window.
    glViewport(0, 0, w, h);
    
    // Switch to the projection matrix.
    glMatrixMode(GL_PROJECTION);
    
    // Reset the projection matrix.
    glLoadIdentity();
    
    // Set up a perspective projection with a field of view of 118 degrees, an aspect ratio of w/h, and a near/far clipping plane of 30.0 and 100.0 respectively.
    gluPerspective(50, (GLfloat)w / h, 1.0, 5000.0);
    
    // Switch back to the modelview matrix.
    glMatrixMode(GL_MODELVIEW);
    
    // Reset the modelview matrix.
    glLoadIdentity();
    
    // Mark the window for redisplay.
    glutPostRedisplay();
}

void GlutFramework::run()
{
    // Enter the GLUT event processing loop
    glutMainLoop();
}