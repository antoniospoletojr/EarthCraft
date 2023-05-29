#include <GL/freeglut.h>
#include "InputHandler.h"
#include "Constants.h"



InputHandler* InputHandler::instance = nullptr;

// Default constructor
InputHandler::InputHandler()
{
    if (InputHandler::instance == nullptr)
        InputHandler::instance = this;
}

// Destructor
InputHandler::~InputHandler()
{
    InputHandler::instance = nullptr;
}

void InputHandler::initialize(Camera *camera, Renderer *renderer)
{
    this->camera = camera;
    this->renderer = renderer;
    
    glutKeyboardFunc(InputHandler::handleRegularKeyPress);
    glutKeyboardUpFunc(InputHandler::handleRegularKeyRelease);
    glutSpecialFunc(InputHandler::handleSpecialKeyPress);
    glutSpecialUpFunc(InputHandler::handleSpecialKeyRelease);
    glutMouseFunc(InputHandler::mouseClick);
    glutMotionFunc(InputHandler::mouseMotion);
    glutIdleFunc(InputHandler::idleCallback);
}

// Handle keyboard input
void InputHandler::handleKeyboard()
{
    // If the splashscreen is not shown then handle the movement keyboard inputs
    if (!(renderer->getCurrentMenuPage() >= 0))
    {
        // Increment the x and z positions. Notice how the cos is 1 when I'm moving on the z axis and the sin is 1 when I'm moving on the x axis.
        if (keys['w'])
            camera->moveForward();
        if (keys['s'])
            camera->moveBackward();
        if (keys['a'])
            camera->moveLeft();
        if (keys['d'])
            camera->moveRight();

        // If Spacebar is pressed then increase the y value of the camera
        if (keys[32])
            camera->moveUp();
        // If left Shift is pressed then decrease the y value of the camera
        if (special_keys[GLUT_KEY_SHIFT_L])
            camera->moveDown();
        
        // Rotate the camera horizontal_angle
        if (special_keys[GLUT_KEY_LEFT])
        camera->rotateLeft();
        if (special_keys[GLUT_KEY_RIGHT])
        camera->rotateRight();
        
        // Rotate the camera vertical_angle
        if (special_keys[GLUT_KEY_UP])
            camera->rotateUp();
        if (special_keys[GLUT_KEY_DOWN])
            camera->rotateDown();
    }
     
     // Exit the program if the Esc key is pressed.
    if (keys[27] || keys['q'])
        exit(0);
    
    // If f is pressed toggle full screen mode on/off
    if (keys['f'])
    {
        is_fullscreen = !is_fullscreen;
        if (is_fullscreen)
            glutFullScreen();
        else
        {
            glutReshapeWindow(960, 540);
            glutPositionWindow(50, 50);
        }
        keys['f'] = false;
    }
    
    // Set the polygon rasterization mode for front and back faces to filled or line mode
    if (keys['p'])
    {
        is_polygon_filled = !is_polygon_filled;
        if (is_polygon_filled)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        keys['p'] = false;
    }
    
    // If enter is pressed travel to the next page in the menu
    if (keys[13])
    {
        
        renderer->incrementMenuPage();
        printf("Enter pressed: %d\n", renderer->getCurrentMenuPage());

        if (renderer->getCurrentMenuPage() == LANDIND_SCREEN)
        {
            // Restore the polygon rasterization mode to filled
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // Set the camera position to the origin
            instance->camera->reset();
        }
        else if (renderer->getCurrentMenuPage() == RIDGES_SCREEN)
        {
            
        }
        else if (renderer->getCurrentMenuPage() == PEAKS_SCREEN)
        {

        }
        else if (renderer->getCurrentMenuPage() == RIVERS_SCREEN)
        {

        }
        else if (renderer->getCurrentMenuPage() == BASINS_SCREEN)
        {
            
        }
        else // Otherwise you are in the rendering screen
        {
            // Set the camera position for the rendering screen
            camera->setPosition(0,500,2000);
        }

        keys[13] = false;
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
    {
        instance->is_mouse_down = false;
    }
}

// Mouse motion callback routine.
void InputHandler::mouseMotion(int x, int y)
{
    // If the rendering screen is shown then update the camera angles based on the mouse movement
    if (instance->renderer->getCurrentMenuPage() == RENDERING_SCREEN)
    {
        if (instance->is_mouse_down)
        {
            // Update the camera horizontal_angle based on the mouse movement
            instance->camera->rotateLeftRight((x - instance->mouse_x)*0.1);
            instance->mouse_x = x;
            
            // Update the camera vertical_angle based on the mouse movement
            instance->camera->rotateUpDown((y - instance->mouse_y)*0.1);
            instance->mouse_y = y;
        }
    }
    
    if (instance->renderer->getCurrentMenuPage() == RIDGES_SCREEN)
    {
        if (instance->is_mouse_down)
        {
            float width = glutGet(GLUT_WINDOW_WIDTH);
            float height = glutGet(GLUT_WINDOW_HEIGHT);
            instance->renderer->sketch(x/width, 1-y/height);
            printf("x: %f, y: %f\n", x / width, 1 - y / height);
        }
    }
}

// Idle function
void InputHandler::idleCallback()
{  
    instance->handleKeyboard();
    glutPostRedisplay();
}