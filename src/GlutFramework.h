/**
@file
@brief GlutFramework header file.
*/

#ifndef GLUTFRAMEWORK_H
#define GLUTFRAMEWORK_H

#include "InputHandler.h"


/**
 * @brief Glut framework class which encapsulates glut statefulness.
 *
 * This class is a singleton that handles the initialization and the encapsulation of the glut framework.
 */
class GlutFramework
{
    public:
        /**
         * @brief Construct a new Glut Framework object.
         * 
         */
        GlutFramework();

        /**
         * @brief Destroy the Glut Framework object
         * 
         */
        ~GlutFramework();
        
        /**
         * @brief Initialize the glut framework.
         * 
         * Many glut initialization functions are called here.
         * 
         * @param argc Number of arguments.
         * @param argv Arguments.
         */
        void initialize(int argc, char** argv);

        /**
         * @brief Resize the glut window.
         * 
         * @param w Width of the window
         * @param h Height of the window 
         */
        static void resize(int w, int h);
        
        /**
         * @brief Run the GLUT event processing loop
         * 
         */
        void run();
    
    private:
        static GlutFramework* instance; ///< Singleton instance of the GlutFramework class.
};

#endif // GLUTFRAMEWORK_H
