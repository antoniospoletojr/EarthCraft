#ifndef GLUTFRAMEWORK_H
#define GLUTFRAMEWORK_H

#include "InputHandler.h"

class GlutFramework
{
    public:
        GlutFramework();
        ~GlutFramework();

        void initialize(int argc, char** argv);
        static void resize(int w, int h);
        void run();
    
    private:
        static GlutFramework* instance;
};

#endif // GLUTFRAMEWORK_H
