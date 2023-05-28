#include <GL/glut.h>
#include <opencv2/opencv.hpp>

GLuint textureID;
cv::VideoCapture splashscreen;
cv::Mat splashscreen_frame;

void loadVideoFrames() {
    splashscreen.open("intro.mp4");
    splashscreen.read(splashscreen_frame);
}

void loadNextFrame(int)
{
    if (!splashscreen.read(splashscreen_frame)) {
        splashscreen.set(cv::CAP_PROP_POS_FRAMES, 0);
        splashscreen.read(splashscreen_frame);
    }

    glutPostRedisplay();
    glutTimerFunc(25, loadNextFrame, 0);  // Delay of 33 milliseconds (about 30 frames per second)
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, splashscreen_frame.cols, splashscreen_frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, splashscreen_frame.data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBegin(GL_QUADS);
            glTexCoord2f(0, 1); glVertex2f(-1, -1);
            glTexCoord2f(1, 1); glVertex2f(1, -1);
            glTexCoord2f(1, 0); glVertex2f(1, 1);
            glTexCoord2f(0, 0); glVertex2f(-1, 1);
        glEnd();
        
        glutSwapBuffers();

}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Window");
    
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &textureID);
    
    loadVideoFrames();
    
    glutDisplayFunc(display);
    glutTimerFunc(0, loadNextFrame, 0);
    glutMainLoop();

    return 0;
}
