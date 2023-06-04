#include <GL/freeglut.h>
#include "InputHandler.h"
#include "Inference.h"
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

void InputHandler::initialize(Camera *camera, Renderer *renderer, SoundManager *sound_manager)
{
    this->camera = camera;
    this->renderer = renderer;
    this->inference = new Inference();
    this->sound_manager = sound_manager;
    this->sound_manager->playBackgroundMusic();

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
    if (!(renderer->current_menu_page >= 0))
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
    {
        exit(0);
    }
    
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
        // Take a snapshot of the current screen before incrementing the menu page
        if (renderer->current_menu_page == RIDGES_SCREEN | renderer->current_menu_page == PEAKS_SCREEN | renderer->current_menu_page == RIVERS_SCREEN | renderer->current_menu_page == BASINS_SCREEN)
            renderer->takeSnapshot();
        
        // If the current page is the last page then go back to the first page
        if (renderer->current_menu_page >= 5)
            renderer->current_menu_page  = -1;
        else
            renderer->current_menu_page++;

        printf("Entering page %d\n", renderer->current_menu_page);

        switch (renderer->current_menu_page)
        {
        case LANDING_SCREEN:
            instance->sound_manager->playResetSound();
            instance->sound_manager->playBackgroundMusic();
            instance->inference->reset();
            instance->camera->reset();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case PEAKS_SCREEN:
        case RIDGES_SCREEN:
        case RIVERS_SCREEN:
        case BASINS_SCREEN:
            instance->sound_manager->playClickSound();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case LOADING_SCREEN:
            instance->sound_manager->playClickSound();
            instance->renderer->resetSketches();
            // Pass the prediction thread the pointer to the boolean variable which tracks the enter key press.
            // When the prediction is complete, the thread simulates an enter key press.
            instance->inference->predict(&keys[13]);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case RENDERING_SCREEN:
            instance->sound_manager->playSuccessSound();
            instance->sound_manager->playBackgroundMusic();
            renderer->initializeMesh();
            camera->setPosition(0, 3000, 2000);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1.0);
            break;
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
    // If the left mouse button is pressed then set the is_mouse_down flag to true and store the mouse coordinates
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        instance->is_mouse_down = true;
        instance->mouse_x = x;
        instance->mouse_y = y;

        short current_page = instance->renderer->current_menu_page;
        if (current_page == RIDGES_SCREEN | current_page == PEAKS_SCREEN | current_page == RIVERS_SCREEN | current_page == BASINS_SCREEN)
        {
            float width = glutGet(GLUT_WINDOW_WIDTH);
            float height = glutGet(GLUT_WINDOW_HEIGHT);
            // If the mouse is outside the sketch area then return
            if (x < width * LEFT_SKETCH_BORDER || x > width * RIGHT_SKETCH_BORDER || y < height * TOP_SKETCH_BORDER || y > height * BOTTOM_SKETCH_BORDER)
                return;
            // Otherwise sketch the pixel
            instance->renderer->sketch(x / width, 1 - y / height);
            // printf("x: %f, y: %f\n", x / width, 1 - y / height);
        }
    }
    // If the left mouse button is released then set the is_mouse_down flag to false
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        instance->is_mouse_down = false;
        short current_page = instance->renderer->current_menu_page;
        if (current_page == RIDGES_SCREEN | current_page == PEAKS_SCREEN | current_page == RIVERS_SCREEN | current_page == BASINS_SCREEN)
        {
            instance->renderer->sketch(0xFFFFFFFFu, 0xFFFFFFFFu);
        }
    }
}

// Mouse motion callback routine.
void InputHandler::mouseMotion(int x, int y)
{
    short current_page = instance->renderer->current_menu_page;
    // If the rendering screen is shown then update the camera angles based on the mouse movement
    if (current_page == RENDERING_SCREEN)
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

    if (current_page == RIDGES_SCREEN | current_page == PEAKS_SCREEN | current_page == RIVERS_SCREEN | current_page == BASINS_SCREEN)
    {
        if (instance->is_mouse_down)
        {
            float width = glutGet(GLUT_WINDOW_WIDTH);
            float height = glutGet(GLUT_WINDOW_HEIGHT);
            // If the mouse is outside the sketch area then return
            if (x < width * LEFT_SKETCH_BORDER || x > width * RIGHT_SKETCH_BORDER || y < height * TOP_SKETCH_BORDER || y > height * BOTTOM_SKETCH_BORDER)
                return;
            // Otherwise sketch the pixel
            instance->renderer->sketch(x/width, 1-y/height);
            //printf("x: %f, y: %f\n", x / width, 1 - y / height);
        }
    }
}

// Idle function
void InputHandler::idleCallback()
{  
    instance->handleKeyboard();
    glutPostRedisplay();
}