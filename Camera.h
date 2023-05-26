#ifndef CAMERA_H
#define CAMERA_H

class Camera
{
    public:
        Camera();
        Camera(GLdouble x, GLdouble y, GLdouble z, GLdouble alfa, GLdouble beta, GLdouble movement_speed, GLdouble rotation_speed);
        ~Camera();
        
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
    
    private:
        GLdouble position[3];
        GLdouble alfa, beta;
        GLdouble movement_speed, rotation_speed;
};

#endif