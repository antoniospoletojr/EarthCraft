/**
@file
@brief Camera source file.
*/

#include "Camera.h"


// Constructor
Camera::Camera()
{
    this->position.x = 0;
    this->position.y = 0;
    this->position.z = 0;
    this->alfa = 0.0f;
    this->beta = 0.0f;
    this->movement_speed = 40.0f;
    this->rotation_speed = 1.5f;
}

// Parametrized constructor
Camera::Camera(GLdouble x, GLdouble y, GLdouble z, GLdouble alfa, GLdouble beta, GLdouble movement_speed, GLdouble rotation_speed)
{
    this->position.x = x;
    this->position.y = y;
    this->position.z = z;
    this->alfa = alfa*(M_PI / 180.0);
    this->beta = beta*(M_PI / 180.0);
    this->movement_speed = movement_speed;
    this->rotation_speed = rotation_speed;
}

// Destructor
Camera::~Camera(){}

void Camera::reset()
{
    this->position.x = 0;
    this->position.y = 0;
    this->position.z = 0;
    this->alfa = 0.0f;
    this->beta = 0.0f;
    this->movement_speed = 4.0f * 10;
    this->rotation_speed = 1.5f;
}

// Set the position of the camera
void Camera::setPosition(GLdouble x, GLdouble y, GLdouble z)
{
    this->position.x = x;
    this->position.y = y;
    this->position.z = z;
}

// Move the camera forward
void Camera::moveForward()
{
    Vec3<float> next_position = this->position;
    next_position.x -= sin(this->alfa) * this->movement_speed;
    next_position.z -= cos(this->alfa) * this->movement_speed;

    bool hasCollided = terrain->checkCollision(next_position);
    if (hasCollided)
        return;
    
    this->position = next_position;
}

// Move the camera backward
void Camera::moveBackward()
{
    Vec3<float> next_position = this->position;
    next_position.x += sin(this->alfa) * this->movement_speed;
    next_position.z += cos(this->alfa) * this->movement_speed;
    
    bool hasCollided = terrain->checkCollision(next_position);
    if (hasCollided)
        return;
    
    this->position = next_position;
}

// Move the camera left
void Camera::moveLeft()
{
    Vec3<float> next_position = this->position;
    next_position.x -= cos(this->alfa) * this->movement_speed;
    next_position.z += sin(this->alfa) * this->movement_speed;

    bool hasCollided = terrain->checkCollision(next_position);
    if (hasCollided)
        return;
    
    this->position = next_position;
}

// Move the camera right
void Camera::moveRight()
{
    Vec3<float> next_position = this->position;
    next_position.x += cos(this->alfa) * this->movement_speed;
    next_position.z -= sin(this->alfa) * this->movement_speed;
    
    bool hasCollided = terrain->checkCollision(next_position);
    if (hasCollided)
        return;
    
    this->position = next_position;
}

// Move the camera up
void Camera::moveUp()
{
    Vec3<float> next_position = this->position;
    next_position.y += this->movement_speed;

    bool hasCollided = terrain->checkCollision(next_position);
    if (hasCollided)
        return;
    
    this->position = next_position;
}

// Move the camera down
void Camera::moveDown()
{
    Vec3<float> next_position = this->position;
    next_position.y -= this->movement_speed;

    bool hasCollided = terrain->checkCollision(next_position);
    if (hasCollided)
        return;

    this->position = next_position;
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

void Camera::increaseSpeed()
{
    this->movement_speed += 10;
}

void Camera::decreaseSpeed()
{
    this->movement_speed -= 10;
}

// Update the camera
void Camera::update()
{
    // Explicitly set gluLookAt() parameters
    GLdouble eye_x = position.x - sin(alfa);
    GLdouble eye_y = position.y;
    GLdouble eye_z = position.z - cos(alfa);
    
    GLdouble center_x = position.x - LOS_DISTANCE * sin(alfa);
    GLdouble center_y = position.y + beta;
    GLdouble center_z = position.z - LOS_DISTANCE * cos(alfa);
    
    GLdouble up_x = 0.0;
    GLdouble up_y = 1.0;
    GLdouble up_z = 0.0;
        
    gluLookAt(eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z);
}

// Returns the direction of the camera in on the x,y,z space
Vec3<float> Camera::getDirection3D()
{
    Vec3<float> eye = (Vec3<float>(position.x - sin(alfa), position.y, position.z - cos(alfa)));
    Vec3<float> center = (Vec3<float>(position.x - LOS_DISTANCE * sin(alfa), position.y + beta, position.z - LOS_DISTANCE * cos(alfa)));
    Vec3<float> difference = subtract(center, eye);
    
    return difference;
}

// Returns the direction of the camera in on the x,z space
Vec2<float> Camera::getDirection2D()
{
    Vec2<float> eye = (Vec2<float>(position.x - sin(alfa), position.z - cos(alfa)));
    Vec2<float> center = (Vec2<float>(position.x - LOS_DISTANCE * sin(alfa), position.z - LOS_DISTANCE * cos(alfa)));
    Vec2<float> difference = subtract(center, eye);
    
    return difference;
}

// Returns the position of the camera in on the x,y,z space
Vec3<float> Camera::getPosition3D()
{
    Vec3<float> coordinates = (Vec3<float>(position.x - sin(alfa), position.y, position.z - cos(alfa)));
    
    return coordinates;
}

// Returns the position of the camera in on the x,z space
Vec2<float> Camera::getPosition2D()
{
    Vec2<float> coordinates = (Vec2<float>(position.x - sin(alfa), position.z - cos(alfa)));
    
    return coordinates;
}

// Set the terrain such that the camera can check for collisions when moving
void Camera::setTerrain(Terrain *terrain)
{
    this->terrain = terrain;
}