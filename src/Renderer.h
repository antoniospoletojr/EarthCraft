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
        short current_menu_page = 0; // keeps track of the current page in the menu
        
        Renderer();
        ~Renderer();
        
        void initialize(Camera *camera);
        void sketch(float x, float y);
        void resetSketches();
        void takeSnapshot();
        void initializeMesh();
        

    private:
        static Renderer* instance;
        Camera *camera;
        Terrain *terrain;
        
        std::vector<Object> objects;
        // Do I need thees as attributes???
        std::vector<GLfloat> mesh_vertices, sun_vertices, moon_vertices, skydome_vertices, sketch_vertices[4];   // an array to keep track of the vertices of the terrain
        std::vector<GLfloat> mesh_colors, sun_colors, moon_colors, skydome_colors, sketch_colors[4]; // an array to keep track of the colors of the terrain
        std::vector<GLuint> mesh_indices, sun_indices, moon_indices, skydome_indices, sketch_indices[4];      // an array to keep track of the indices of the terrain
        std::vector<GLfloat> mesh_textures, sun_textures, moon_textures, skydome_textures;
        
        cv::VideoCapture menu_clips[6];
        cv::Mat menu_frame;

        float angle = 0.0f; // to rename to something more meaningful (date)
        
        void initializeOrbit();
        void initializeSkydome();
        void initializeSplashscreen();
        void initializeCanvas();

        void cycleDayNight();

        static void drawMesh();
        static void drawOrbit();
        static void drawSkydome();
        static void drawSplashscreen();
        static void drawCanvas();
        static void drawSketch(short current_canvas);
        
        static void timerCallback(int value);
        static void draw();
};

#endif