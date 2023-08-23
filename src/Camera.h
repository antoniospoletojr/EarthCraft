/**
@file
@brief Camera source file.
*/

#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glut.h>
#include <stdio.h>
#include <cmath>
#include "Constants.h"
#include "Vec.hpp"
#include "Terrain.h"

/**
* @brief Camera management class
*
* Implement camera movement and rotation and checks for collisions.
*/
class Camera
{
    public:
        /**
         * @brief Default constructor.
         */
        Camera();
        
        /**
         * @brief Parametrized constructor.
         * @param x Initial x-coordinate of the camera.
         * @param y Initial y-coordinate of the camera.
         * @param z Initial z-coordinate of the camera.
         * @param alfa Initial horizontal angle of the camera (in degrees).
         * @param beta Initial vertical angle of the camera (in degrees).
         * @param movement_speed Speed at which the camera moves.
         * @param rotation_speed Speed at which the camera rotates.
         */
        Camera(GLdouble x, GLdouble y, GLdouble z, GLdouble alfa, GLdouble beta, GLdouble movement_speed, GLdouble rotation_speed);
        
        /**
         * @brief Destructor.
         */
        ~Camera();
        
        /**
         * @brief Reset the camera to its initial position.
         */
        void reset();

        /**
         * @brief Set the position of the camera.
         * @param x New x-coordinate of the camera.
         * @param y New y-coordinate of the camera.
         * @param z New z-coordinate of the camera.
         */
        void setPosition(GLdouble x, GLdouble y, GLdouble z);

        /**
         * @brief Move the camera forward.
         */
        void moveForward();

        /**
         * @brief Move the camera backward.
         */
        void moveBackward();

        /**
         * @brief Move the camera left.
         */
        void moveLeft();

        /**
         * @brief Move the camera right.
         */
        void moveRight();

        /**
         * @brief Move the camera up.
         */
        void moveUp();

        /**
         * @brief Move the camera down.
         */
        void moveDown();

        /**
         * @brief Rotate the camera left.
         */
        void rotateLeft();

        /**
         * @brief Rotate the camera right.
         */
        void rotateRight();

        /**
         * @brief Rotate the camera up.
         */
        void rotateUp();

        /**
         * @brief Rotate the camera down.
         */
        void rotateDown();

        /**
         * @brief Rotate the camera left and up.
         */
        void rotateLeftRight(GLdouble delta);

        /**
         * @brief Rotate the camera left and down.
         */
        void rotateUpDown(GLdouble delta);

        /**
         * @brief Increase the speed of the camera.
         */
        void increaseSpeed();

        /**
         * @brief Decrease the speed of the camera.
         */
        void decreaseSpeed();

        /**
         * @brief Update the camera position.
         */
        void update();
        
        /**
         * @brief Get the direction of the camera in the 3D space (x,y,z).
         * @return The direction of the camera.
         */
        Vec3<float> getDirection3D();

        /**
         * @brief Get the direction of the camera in the 2D space (x,z).
         * @return The direction of the camera.
         */
        Vec2<float> getDirection2D();

        /**
         * @brief Get the position of the camera in the 3D space (x,y,z).
         * @return The position of the camera.
         */
        Vec3<float> getPosition3D();

        /**
         * @brief Get the position of the camera in the 2D space (x,z).
         * @return The position of the camera.
         */
        Vec2<float> getPosition2D();

        /**
         * @brief Set the terrain of the camera to check for collisions.
         */
        void setTerrain(Terrain *terrain);
    
    private:
        Vec3<float> position;                       ///< Camera position (x, y, z).
        GLdouble alfa, beta;                        ///< Camera angles (horizontal, vertical).
        GLdouble movement_speed, rotation_speed;    ///< Camera speeds (movement, rotation).
        Terrain *terrain;                           ///< Terrain object reference for collision checks.
};

#endif