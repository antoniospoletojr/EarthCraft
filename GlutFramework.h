#ifndef GLUTFRAMEWORK_H
#define GLUTFRAMEWORK_H

#include "InputHandler.h"

class GlutFramework
{
    public:
        GlutFramework();
        ~GlutFramework();

        void initialize(int argc, char** argv, InputHandler* input_handler);
        static void resize(int w, int h);
        static void update();
        void run();
    
    private:
        static GlutFramework* instance;
        InputHandler* input_handler;
};

#endif // GLUTFRAMEWORK_H
