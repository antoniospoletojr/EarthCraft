#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "Camera.h"
#include "Renderer.h"

class InputHandler
{
    public:
        InputHandler();
        ~InputHandler();
        
        void initialize(Camera *camera, Renderer *renderer);
        void handleKeyboard();
        static void handleRegularKeyPress(unsigned char key, int x, int y);
        static void handleRegularKeyRelease(unsigned char key, int x, int y);
        static void handleSpecialKeyPress(int key, int x, int y);
        static void handleSpecialKeyRelease(int key, int x, int y);
        static void mouseClick(int button, int state, int x, int y);
        static void mouseMotion(int x, int y);
        static void idleCallback();
    
    private:
        static InputHandler *instance;  // used as a trick to access the InputHandler object from the static callback functions
        Camera *camera;                 // a reference to the camera object
        Renderer *renderer;             // a reference to the renderer object
        
        bool keys[256];                 // an array to keep track of regular key presses
        bool special_keys[256];         // an array to keep track of special key presses
        bool is_mouse_down = false;     // keeps track of whether or not the mouse is down
        GLint mouse_x;                  // keeps track of the x position of the mouse
        GLint mouse_y;                  // keeps track of the y position of the mouse
        bool is_polygon_filled = false; // keeps track of whether or not the polygon is filled
        bool is_fullscreen = true;      // keeps track of whether or not the window is in is_fullscreen mode
};

#endif // INPUTHANDLER_H