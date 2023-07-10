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
    GLuint vao;
    GLuint vbo;
    GLuint tbo;
    GLuint ibo;
    GLuint cbo;
    GLuint nbo;
    GLuint texture;
    GLuint blend_texture;
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
    void initializeMesh(Terrain *terrain);

private:
    static Renderer *instance;
    Camera *camera;
    Terrain *terrain;

    std::vector<Object> objects;
    std::vector<GLfloat> sketch_vertices[4];  // an array to keep track of the vertices of the terrain
    std::vector<GLfloat> sketch_colors[4];                                                  // an array to keep track of the colors of the terrain
    std::vector<GLuint> sketch_indices[4];        // an array to keep track of the indices of the terrain
    
    cv::VideoCapture menu_clips[6];
    cv::Mat menu_frame;
    cv::Mat day_texture;
    cv::Mat night_texture;
    
    float time = 12.0f; // time variable used to track time of the day and apply corresponding rotation to the sun and moon

    void initializeOrbit();
    void initializeSkydome();
    void initializeSplashscreen();
    void initializeCanvas();

    void cycleDayNight();

    static void drawMesh(int mesh_multiplier);
    static void drawOrbit();
    static void drawSkydome();
    static void drawSplashscreen();
    static void drawCanvas();
    static void drawSketch(short current_canvas);
    static void drawTime();

    static void timerCallback(int value);
    static void draw();
};

#endif