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

    // Set the clear color for the color buffer to white with 0 alpha (fully opaque)
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    // Set polygon mode to be not filled
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Set the culling mode to be back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
        
    // Enable the depth test to ensure that polygons that are behind others are not drawn
    glEnable(GL_DEPTH_TEST);

    // Enable 2D texturing
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_COLOR_MATERIAL);
    
    // Turn on OpenGL lighting.
    glEnable(GL_LIGHT0);                                // Enable spotlight diffuse light source.
    glEnable(GL_LIGHT1);                                // Enable directional ambient light source.
    
    // Light property vectors.
    float ambient_light[] = {1, 1, 1, 1.0};
    
    // Light properties.
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient_light);
    
    //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        
    // Set the cutoff angle to a larger value for wider light rays
    
    
    // float constantAttenuation = 1.0f;
    // float linearAttenuation = 1.0f;
    // float quadraticAttenuation = 1.0f;
    
    // glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constantAttenuation);
    // glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linearAttenuation);
    // glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadraticAttenuation);

    // Enable automatic normalization of surface normals to unit length
    glEnable(GL_NORMALIZE);
    
    // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    gluPerspective(50, (GLfloat)w / h, 1.0, 100000.0);
    
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