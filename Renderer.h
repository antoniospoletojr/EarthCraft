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
} Object;

class Renderer
{
    public:
        Renderer();
        ~Renderer();
        
        void initialize(Terrain *terrain, Camera *camera);
        void toggleSplashscreen();

        
    private:
        static Renderer* instance;
        Camera *camera;
        Terrain *terrain;
        
        std::vector<Object> objects;
        // Do I need thees as attributes???
        std::vector<float> mesh_vertices, sun_vertices, planet_vertices;    // an array to keep track of the vertices of the terrain
        std::vector<float> mesh_colors, sun_colors, planet_colors;          // an array to keep track of the colors of the terrain
        std::vector<int> mesh_indices, sun_indices, planet_indices;         // an array to keep track of the indices of the terrain
        
        float mesh_dim;
        bool splashscreen_playing = true;

        GLuint textureID;
        cv::VideoCapture splashscreen;
        cv::Mat splashscreen_frame;

        void initializeMesh();
        void initializeSun();
        void initializeSplashscreen();
        void moveSun();
        static void drawMesh();
        static void drawSun();
        static void drawSplashscreen();
        static void loadVideoFrames();
        static void timerCallback(int value);
        static void draw();
};

#endif