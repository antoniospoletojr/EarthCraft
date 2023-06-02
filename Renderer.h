#ifndef RENDERER_H
#define RENDERER_H

#include "Camera.h"
#include "Terrain.h"
#include <opencv2/opencv.hpp>
#include <vector>

typedef struct Object
{
    GLuint vao;
    GLuint vbo;
    GLuint tbo;
    GLuint ibo;
    GLuint cbo;
    GLuint texture;
} Object;

class Renderer
{
    public:
        Renderer();
        ~Renderer();
        
        void initialize(Terrain *terrain, Camera *camera);
        void incrementMenuPage();
        short getCurrentMenuPage();
        void sketch(float x, float y);
        void takeSnapshot();

    private:
        static Renderer* instance;
        Camera *camera;
        Terrain *terrain;
        
        std::vector<Object> objects;
        // Do I need thees as attributes???
        std::vector<GLfloat> mesh_vertices, sun_vertices, planet_vertices, sketch_vertices;   // an array to keep track of the vertices of the terrain
        std::vector<GLfloat> mesh_colors, sun_colors, planet_colors, sketch_colors;           // an array to keep track of the colors of the terrain
        std::vector<GLuint> mesh_indices, sun_indices, planet_indices, sketch_indices;      // an array to keep track of the indices of the terrain

        float mesh_dim;

        // Menu variables and assets
        short current_menu_page = 0; // keeps track of the current page in the menu
        cv::VideoCapture menu_clips[5];
        cv::Mat menu_frame;
        
        void initializeMesh();
        void initializeSun();
        void initializeSplashscreen();
        void initializeCanvas();
        void initializeSketch();
        
        void moveSun();

        static void drawMesh();
        static void drawSun();
        static void drawSplashscreen();
        static void drawCanvas();
        static void drawSketch();
        
        static void timerCallback(int value);
        static void draw();
};

#endif