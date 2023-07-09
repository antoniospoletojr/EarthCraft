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
    std::vector<GLuint> indices;
    std::vector<GLfloat> textures;
    std::vector<GLfloat> normals;
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
    // Do I need thees as attributes???
    std::vector<GLfloat> mesh_vertices, sun_vertices, moon_vertices, skydome_vertices, sketch_vertices[4];  // an array to keep track of the vertices of the terrain
    std::vector<GLfloat> skydome_colors, sketch_colors[4];                                                  // an array to keep track of the colors of the terrain
    std::vector<GLuint> mesh_indices, sun_indices, moon_indices, skydome_indices, sketch_indices[4];        // an array to keep track of the indices of the terrain
    std::vector<GLfloat> mesh_textures, sun_textures, moon_textures, skydome_textures;
    std::vector<GLfloat> mesh_normals;
    
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