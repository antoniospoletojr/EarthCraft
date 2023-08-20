#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include "random"

#include "Camera.h"
#include "Object.h"
#include "QuadTree.h"
#include "Terrain.h"
#include "Constants.h"
#include "Vec.hpp"
#include "PerlinNoise.hpp"


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
    void initializeWater();
    void initializeOrbit(int orbit_height);
    void initializeVegetation();
    void setTerrain(Terrain *terrain);

private:
    static Renderer *instance;
    Camera *camera;
    Terrain *terrain;
    QuadTree quadtree;
    
    std::vector<Object> objects;
    
    cv::VideoCapture menu_clips[6];
    cv::Mat menu_frame;
    
    float time = 12.f; // time variable used to track time of the day and apply corresponding rotation to the sun and moon
    
    siv::PerlinNoise perlin_noise;
    
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
    static void drawWater();
    static void drawVegetation();
    static void renderLight();
    
    static void timerCallback(int value);
    static void draw();
};

#endif