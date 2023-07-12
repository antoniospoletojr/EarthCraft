#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include "random"

#include "Camera.h"
#include "Terrain.h"
#include "Constants.h"
#include "Vec.hpp"

typedef struct Object
{
    // Buffer object descriptors
    GLuint vao;
    GLuint vbo;
    GLuint tbo;
    GLuint ibo;
    GLuint cbo;
    GLuint nbo;
    GLuint texture;
    GLuint blend_texture;
    
    // Buffers
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> textures;
    std::vector<GLuint> indices;
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
    void initializeMesh(Terrain *terrain, int mesh_multiplier);

private:
    static Renderer *instance;
    Camera *camera;
    Terrain *terrain;
    
    std::vector<Object> objects;
    
    cv::VideoCapture menu_clips[6];
    cv::Mat menu_frame;
    
    cv::Mat day_texture;
    cv::Mat night_texture;
    
    float time = 12.f; // time variable used to track time of the day and apply corresponding rotation to the sun and moon
    
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
    static void drawTime();
    static void renderLight();

    static void timerCallback(int value);
    static void draw();
};

#endif