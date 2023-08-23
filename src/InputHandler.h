/**
@file
@brief InputHandler header file.
*/

#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "Renderer.h"
#include "Camera.h"
#include "Colors.h"
#include "Constants.h"
#include "Inference.h"
#include "SoundManager.h"
#include <GL/freeglut.h>
#include <thread>

/**
 * @brief Input handler class which handles the user input.
 *
 * This class handles the user input and manages the callbacks for the keyboard, mouse, idle and sound events together with callbacks from other objects.
 */
class InputHandler
{
    public:
        /**
         * @brief Construct a new Input Handler object.
         * 
         */
        InputHandler();
        /**
         * @brief Destroy the Input Handler object
         * 
         */
        ~InputHandler();
        
        /**
         * @brief 
         * 
         * Initializes the input handler by setting the camera, renderer, sound manager, and quadtree references.
         * Also sets the glut callbacks for the events.
         * 
         * @param camera Reference to the camera object
         * @param renderer Reference to the renderer object
         * @param sound_manager Reference to the sound manager object
         * @param quadtree Reference to the quadtree object
         * 
         */
        void initialize(Camera *camera, Renderer *renderer, SoundManager *sound_manager, QuadTree *quadtree);

    private:
        static InputHandler *instance;           ///< Used to access the InputHandler object from the static callback functions
        Camera *camera;                          ///< a reference to the camera object
        Renderer *renderer;                      ///< a reference to the renderer object
        Inference *inference;                    ///< a reference to the inference object
        SoundManager *sound_manager;             ///< a reference to the sound engine object
        Terrain *terrain;
        QuadTree *quadtree;

        bool keys[256];                     ///< an array to keep track of regular key presses
        bool special_keys[256];             ///< an array to keep track of special key presses
        bool is_mouse_down = false;         ///< keeps track of whether or not the mouse is down
        GLint mouse_x;                      ///< keeps track of the x position of the mouse
        GLint mouse_y;                      ///< keeps track of the y position of the mouse
        bool is_polygon_filled = true;      ///< keeps track of whether or not the polygon is filled
        bool is_fullscreen = true;          ///< keeps track of whether or not the window is in is_fullscreen mode

        std::thread generation_thread;      ///< handles the input event associated to generation of the terrain in a separate thread
        
        
        /**
         * @brief Handles the keyboard input.
         * 
         * Checks if the user pressed a key and calls the appropriate function.
         * 
         */
        void handleKeyboard();

        /**
         * @brief Handles the regular key press by setting the corresponding key in the keys array to true.
         * 
         * @param key Key pressed
         * @param x x position of the key
         * @param y y position of the mouse
         */
        static void handleRegularKeyPress(unsigned char key, int x, int y);

        /**
         * @brief Handles the regular key release by setting the corresponding key in the keys array to false.
         * 
         * @param key Key released
         * @param x x position of the mouse
         * @param y y position of the mouse
         */
        static void handleRegularKeyRelease(unsigned char key, int x, int y);

        /**
         * @brief Handles the special key press by setting the corresponding key in the special_keys array to true.
         * 
         * @param key Key pressed
         * @param x x position of the mouse
         * @param y y position of the mouse
         */
        static void handleSpecialKeyPress(int key, int x, int y);

        /**
         * @brief Handles the special key release by setting the corresponding key in the special_keys array to false.
         * 
         * @param key Key released
         * @param x x position of the mouse
         * @param y y position of the mouse
         */
        static void handleSpecialKeyRelease(int key, int x, int y);

        /**
         * @brief Handles the mouse click by setting the is_mouse_down variable to true and affecting the sketching on the canvas.
         * 
         * @param button Button pressed
         * @param state State of the button (pressed, released, etc.)
         * @param x x position of the mouse
         * @param y y position of the mouse
         */
        static void mouseClick(int button, int state, int x, int y);

        /**
         * @brief Handles the mouse motion by setting the mouse_x and mouse_y variables to the current mouse position and affecting the sketching on the canvas.
         * 
         * @param x x position of the mouse
         * @param y y position of the mouse
         */
        static void mouseMotion(int x, int y);

        /**
         * @brief Idle callback function which is called when the program is idle and checks if the user pressed a key.
         * 
         */
        static void idleCallback();

        /**
         * @brief Implements the generation of the terrain in a separate thread as a consequence of the user key press.
         * 
         */
        static void generate();
};

#endif // INPUTHANDLER_H