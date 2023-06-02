#include <GL/glut.h>
#include <iostream>

int windowWidth = 0;
int windowHeight = 0;

void reshape(int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    std::cout << "Width: " << windowWidth << ", Height: " << windowHeight << std::endl;
}

void toggleFullscreen()
{
    if (!glutGet(GLUT_FULLY_COVERED))
    {
        glutFullScreen();
    }
    else
    {
        glutReshapeWindow(800, 600);  // Set the desired window size
        glutPositionWindow(100, 100); // Set the desired window position
    }
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 'f' || key == 'F')
    {
        toggleFullscreen();
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Window");

    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
