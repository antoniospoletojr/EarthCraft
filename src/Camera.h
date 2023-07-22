#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glut.h>
#include <stdio.h>
#include <cmath>
#include "Constants.h"
#include "Vec.hpp"
#include "Terrain.h"

class Camera
{
    public:
        Camera();
        Camera(GLdouble x, GLdouble y, GLdouble z, GLdouble alfa, GLdouble beta, GLdouble movement_speed, GLdouble rotation_speed);
        ~Camera();
        
        void reset();
        void setPosition(GLdouble x, GLdouble y, GLdouble z);
        void moveForward();
        void moveBackward();
        void moveLeft();
        void moveRight();
        void moveUp();
        void moveDown();
        void rotateLeft();
        void rotateRight();
        void rotateUp();
        void rotateDown();
        void rotateLeftRight(GLdouble delta);
        void rotateUpDown(GLdouble delta);
        void update();

        Vec3<float> getDirection();
        Vec3<float> getPosition();
        void setTerrain(Terrain *terrain);
    
    private:
        Vec3<float> position;
        GLdouble alfa, beta;
        GLdouble movement_speed, rotation_speed;
        Terrain *terrain;
};

#endif