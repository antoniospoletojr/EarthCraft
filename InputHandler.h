#include <GL/glut.h>
#include "Camera.h"

class InputHandler
{
    public:
        InputHandler();
        InputHandler(Camera& camera);
        
        void handleKeyboard(unsigned char key, int x, int y);
        
        void handleMouse(int button, int state, int x, int y);
        
        void handleMouseMotion(int x, int y);

    private:
        Camera& camera;
};