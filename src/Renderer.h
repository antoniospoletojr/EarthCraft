/**
@file
@brief Renderer header file.
*/

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

/**
 * @brief Renderer class which handles the rendering of the scene and all of the objects.
 * 
 * Handles the rendering of menus, sketches, objects, lighting and real time calculations (update to normals, water physics, vegetation movement, etc.  )
 */
class Renderer
{
public:
    short current_menu_page = 0; ///< Keeps track of the current page in the menu
    
    /**
     * @brief Renderer singleton constructor.
     */
    Renderer();
    
    /**
     * @brief Renderer singleton destructor.
     */
    ~Renderer();
    
    /**
     * @brief Initialize the renderer.
     * 
     * Set the camera and quadtree references, allocate space for objects, set the timer func and populate fixed objects (skydome, splashscreen and canvas).
     * 
     * @param camera Camera object reference
     * @param quadtree Quadtree object reference
     */
    void initialize(Camera *camera, QuadTree *quadtree);

    /**
     * @brief Take a snapshot of the current canvas.
     * 
     * This function takes a snapshot of the current canvas by rendering the scene to a framebuffer object
     * independent of the current window (so that window stretch doesn't affect the snapshot) and saves it to the disk into the assets/sketches folder.
     */
    void takeSnapshot();

    /**
     * @brief Initialize the water object.
     */
    void initializeWater();

    /**
     * @brief Initialize the orbit object consisting of a sun and of a moon rotating in an orbit.
     * 
     * @param orbit_height Orbit height in world coordinates calculated as a function of the map size
     */
    void initializeOrbit(int orbit_height);

    /**
     * @brief Initialize the vegetation objects consting of different random located bushes.
     */
    void initializeVegetation();

    /**
     * @brief Set the Terrain object reference.
     * 
     * @param terrain Terrain object reference
     */
    void setTerrain(Terrain *terrain);

    /**
     * @brief Set the time variable.
     * 
     * @param time New time
     */
    void setTime(int time);

    /**
     * @brief Add new drawn pixels to the current canvas.
     * 
     * @param x x-coordinate of the pixel
     * @param y y-coordinate of the pixel
     */
    void sketch(float x, float y);

    /**
     * @brief Clear the sketches buffer (used when restarting the loop).
     */
    void resetSketches();

private:
    static Renderer *instance;          ///< Used to access the Renderer object from the static callback functions
    Camera *camera;                     ///< A reference to the camera object
    Terrain *terrain;                   ///< A reference to the terrain object
    QuadTree *quadtree;                 ///< A reference to the quadtree object
    std::vector<Object> objects;        ///< The vector of objects to be rendered
    cv::VideoCapture menu_clips[6];     ///< The vector of menu clips to be rendered
    cv::Mat menu_frame;                 ///< The current menu frame to be rendered
    float time;                         ///< Time variable used to track time of the day and apply time-based effects
    siv::PerlinNoise perlin_noise;      ///< Perlin noise object used to generate the water waves
    
    /**
     * @brief Initialize the skydome object.
     */
    void initializeSkydome();

    /**
     * @brief Initialize the splashscreen object.
     */
    void initializeSplashscreen();

    /**
     * @brief Initialize the canvas object.
     */
    void initializeCanvas();

    /**
     * @brief Update the time continuosly.
     */
    void cycleDayNight();

    /**
     * @brief Draw the terrain mesh by means of the quadtree.
     */
    static void drawTerrain();

    /**
     * @brief Draw the orbit and handles sun and moon rotation.
     */
    static void drawOrbit();

    /**
     * @brief Draw the skydome and handle day and night blending.
     */
    static void drawSkydome();

    /**
     * @brief Draw the splashscreen.
     */
    static void drawSplashscreen();

    /**
     * @brief Draw the canvas.
     */
    static void drawCanvas();

    /**
     * @brief Draw the current canvas with the current drawn sketch frame.
     * 
     * @param current_canvas The current canvas to be drawn
     */
    static void drawSketch(short current_canvas);

    /**
     * @brief Draw the time text on the top center of the screen.
     */
    static void drawTime();

    /**
     * @brief Draw the water and handle its realtime simualted physics and reflections.
     */
    static void drawWater();

    /**
     * @brief Draw the vegetation and handle the realtime simualted wind blowing on the bushes.
     */
    static void drawVegetation();

    /**
     * @brief Handle overall lighting its parameters.
     */
    static void renderLight();
    
    /**
     * @brief Animate menu frames.
     */
    static void animateMenu();
    
    /**
     * @brief Implements the timer function which periodically animates the menus and carries the time flowing.
     * 
     * @param value Timer value (unused)
     */
    static void timerCallback(int value);

    /**
     * @brief Draw the entire scene.
     */
    static void draw();
};

#endif // RENDERER_H
