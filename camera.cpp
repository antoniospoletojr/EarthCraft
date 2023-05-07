#include <GL/glut.h>
#include <GL/freeglut.h>
#include <cmath>
#include "camera.h"

// Constructor
Camera::Camera()
{
    position[0] = 0;
    position[1] = 30;
    position[2] = 900;
    alfa = 0.0f;
    beta = 0.0f;
    movement_speed = 4.0f;
    rotation_speed = 2.0f;
}

// Parametrized constructor
Camera::Camera(GLdouble x, GLdouble y, GLdouble z, GLdouble alfa, GLdouble beta, GLdouble movement_speed, GLdouble rotation_speed)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
    this->alfa = alfa*(M_PI / 180.0);
    this->beta = beta*(M_PI / 180.0);
    this->movement_speed = movement_speed;
    this->rotation_speed = rotation_speed;
}

// Destructor
Camera::~Camera()
{

}

// Move the camera forward
void Camera::moveForward()
{
    position[0] -= sin(alfa * M_PI / 180.0) * movement_speed;
    position[2] -= cos(alfa * M_PI / 180.0) * movement_speed;
}

// Move the camera backward
void Camera::moveBackward()
{
    position[0] += sin(alfa * M_PI / 180.0) * movement_speed;
    position[2] += cos(alfa * M_PI / 180.0) * movement_speed;
}

// Move the camera left
void Camera::moveLeft()
{
    position[0] -= cos(alfa * M_PI / 180.0) * movement_speed;
    position[2] += sin(alfa * M_PI / 180.0) * movement_speed;
}

// Move the camera right
void Camera::moveRight()
{
    position[0] += cos(alfa * M_PI / 180.0) * movement_speed;
    position[2] -= sin(alfa * M_PI / 180.0) * movement_speed;
}

// Move the camera up
void Camera::moveUp()
{
    position[1] += movement_speed;
}

// Move the camera down
void Camera::moveDown()
{
    if (position[1] > 1)
        position[1] -= movement_speed;
}

// Rotate the camera left
void Camera::rotateLeft()
{
    alfa += (M_PI / 180.0)*rotation_speed;
    if (alfa > 2*M_PI)
        alfa -= 2*M_PI;
}

// Rotate the camera right
void Camera::rotateRight()
{
    alfa -= (M_PI / 180.0)*rotation_speed;
    if (alfa < 0.0)
        alfa += 2*M_PI;
}

// Rotate the camera up
void Camera::rotateUp()
{   
    if (beta < 180.0)
        beta += rotation_speed;
}

// Rotate the camera down
void Camera::rotateDown()
{
    if (beta > -180.0)
        beta -= rotation_speed;
}

// Update the camera
void Camera::update()
{
    gluLookAt(position[0] - 1 * sin(alfa), position[1] , position[2] - 1 * cos(alfa),
              position[0] - 2 * sin(alfa), position[1] + 0.01 * beta, position[2] - 2 * cos(alfa),
              0.0, 1.0, 0.0);
}