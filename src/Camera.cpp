#include "Camera.h"

// Constructor
Camera::Camera()
{
    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    alfa = 0.0f;
    beta = 0.0f;
    movement_speed = 4.0f * 6;
    rotation_speed = 1.5f;
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
Camera::~Camera(){}

void Camera::reset()
{
    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    alfa = 0.0f;
    beta = 0.0f;
    movement_speed = 4.0f;
    rotation_speed = 1.5f;
}

// Set the position of the camera
void Camera::setPosition(GLdouble x, GLdouble y, GLdouble z)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
}

Vertex3d<float> Camera::getPosition()
{
    Vertex3d<float> position;
    position.x = this->position[0] - sin(alfa);
    position.y = this->position[1];
    position.z = this->position[2] - cos(alfa);
    return position;
}

Vertex3d<float> Camera::getDirection()
{
    Vertex3d<float> direction;
    direction.x = position[0] - LOS_DISTANCE * sin(alfa);
    direction.y = position[1] + beta;
    direction.z = position[2] - LOS_DISTANCE * cos(alfa);
    return direction;
}

// Move the camera forward
void Camera::moveForward()
{
    position[0] -= sin(alfa) * movement_speed;
    position[2] -= cos(alfa) * movement_speed;
}

// Move the camera backward
void Camera::moveBackward()
{
    position[0] += sin(alfa) * movement_speed;
    position[2] += cos(alfa) * movement_speed;
}

// Move the camera left
void Camera::moveLeft()
{
    position[0] -= cos(alfa ) * movement_speed;
    position[2] += sin(alfa) * movement_speed;
}

// Move the camera right
void Camera::moveRight()
{
    position[0] += cos(alfa) * movement_speed;
    position[2] -= sin(alfa) * movement_speed;
}

// Move the camera up
void Camera::moveUp()
{
    position[1] += movement_speed;
}

// Move the camera down
void Camera::moveDown()
{
    if (position[1] > 100)
        position[1] -= movement_speed;
}

// Rotate the camera left
void Camera::rotateLeft()
{
    alfa += (M_PI / 180.0) * rotation_speed;
    if (alfa > 2 * M_PI)
        alfa -= 2 * M_PI;
}

// Rotate the camera right
void Camera::rotateRight()
{
    alfa -= (M_PI / 180.0) * rotation_speed;
    if (alfa < 0.0)
        alfa += 2 * M_PI;
}

// Rotate the camera up
void Camera::rotateUp()
{       
    beta += (M_PI / 180.0) * rotation_speed;
}

// Rotate the camera down
void Camera::rotateDown()
{
    beta -= (M_PI / 180.0) * rotation_speed;
}

void Camera::rotateLeftRight(GLdouble delta)
{
    alfa -= delta*(M_PI / 180.0);
}

void Camera::rotateUpDown(GLdouble delta)
{
    beta -= delta*(M_PI / 180.0);
}

// Update the camera
void Camera::update()
{
    // Explicitly set gluLookAt() parameters
    GLdouble eyeX = position[0] - sin(alfa);
    GLdouble eyeY = position[1];
    GLdouble eyeZ = position[2] - cos(alfa);
    
    GLdouble centerX = position[0] - LOS_DISTANCE * sin(alfa);
    GLdouble centerY = position[1] + beta;
    GLdouble centerZ = position[2] - LOS_DISTANCE * cos(alfa);
    
    GLdouble upX = 0.0;
    GLdouble upY = 1.0;
    GLdouble upZ = 0.0;
    
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
}