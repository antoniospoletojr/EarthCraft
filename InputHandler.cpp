#include <GL/glut.h>
#include "InputHandler.h"

// Default constructor
InputHandler::InputHandler()
{
    #warning "InputHandler being called without a Camera object."
}

// Parametrized constructor
InputHandler::InputHandler(Camera& camera)
{
    this->camera = camera;
}

// Destructor
InputHandler::~InputHandler()
{

}

// Handle keyboard input
void InputHandler::handleKeyboard(unsigned char key, int x, int y)
{
    switch(key) {
        // Handle keyboard input here
        case 'w':
            camera.moveForward();
            break;
        case 's':
            camera.moveBackward();
            break;
        case 'a':
            camera.moveLeft();
            break;
        case 'd':
            camera.moveRight();
            break;
        default:
            break;
    }
}
