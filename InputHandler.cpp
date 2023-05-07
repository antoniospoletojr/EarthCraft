#include <GL/glut.h>
#include <GL/freeglut.h>
#include "InputHandler.h"

InputHandler* InputHandler::instance = nullptr;

// Parametrized constructor
InputHandler::InputHandler(Camera& camera):camera(camera)
{
    InputHandler::instance = this;
}

// Destructor
InputHandler::~InputHandler()
{
    InputHandler::instance = nullptr;
}

void InputHandler::init()
{
    glutKeyboardFunc(InputHandler::handleRegularKeyPress);
    glutKeyboardUpFunc(InputHandler::handleRegularKeyRelease);
    glutSpecialFunc(InputHandler::handleSpecialKeyPress);
    glutSpecialUpFunc(InputHandler::handleSpecialKeyRelease);
    glutMouseFunc(InputHandler::mouseClick);
    glutMotionFunc(InputHandler::mouseMotion);
}

// Handle keyboard input
void InputHandler::handleKeyboard()
{
    // Increment the x and z positions. Notice how the cos is 1 when I'm moving on the z axis and the sin is 1 when I'm moving on the x axis.
    if (keys['w'])
    {
        camera.moveForward();
    }
    if (keys['s'])
        camera.moveBackward();
    if (keys['a'])
        camera.moveLeft();
    if (keys['d'])
        camera.moveRight();

    // If Spacebar is pressed then increase/decrease the y value of the camera
    if (keys[32])
    {
        if(!special_keys[GLUT_KEY_SHIFT_L])
            camera.moveUp();
        else
            camera.moveDown();
    }
    
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
}

void InputHandler::handleRegularKeyPress(unsigned char key, int x, int y)
{
    instance->keys[key] = true;
}

void InputHandler::handleRegularKeyRelease(unsigned char key, int x, int y)
{
    instance->keys[key] = false;
}

void InputHandler::handleSpecialKeyPress(int key, int x, int y)
{
    instance->special_keys[key] = true;
}

void InputHandler::handleSpecialKeyRelease(int key, int x, int y)
{
    instance->special_keys[key] = false;
}

void InputHandler::mouseClick(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        instance->is_mouse_down = true;
        instance->mouse_x = x;
        instance->mouse_y = y;
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
        instance->is_mouse_down = false;

    glutPostRedisplay();
}

// Mouse motion callback routine.
void InputHandler::mouseMotion(int x, int y)
{
    if (instance->is_mouse_down)
    {
        // Update the camera horizontal_angle based on the mouse movement
        instance->camera.rotateLeftRight((x - instance->mouse_x)*0.1);
        instance->mouse_x = x;
        
        // Update the camera vertical_angle based on the mouse movement
        instance->camera.rotateUpDown((y - instance->mouse_y)*0.1);
        instance->mouse_y = y;
    }
    //glutPostRedisplay();
}
