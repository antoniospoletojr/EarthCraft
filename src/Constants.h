/**
@file
@brief Constants macros.
*/

// Terrain macros
#define WORLD_SCALE 150
#define TEXTURE_SCALE 20
#define FLOODING_FACTOR 0.15

// Renderer macros
#define RENDERING_SCREEN -1
#define LANDING_SCREEN 0
#define RIDGES_SCREEN 1
#define PEAKS_SCREEN 2
#define RIVERS_SCREEN 3
#define BASINS_SCREEN 4
#define LOADING_SCREEN 5

#define RIDGES 0
#define PEAKS 1
#define RIVERS 2
#define BASINS 3

#define SPLASHSCREEN 0
#define CANVAS 1
#define SKYDOME 2
#define SUN 3
#define MOON 4
#define WATER 5
#define VEGETATION 6
#define SKETCH 7

#define VEGETATION_SPARSITY 50
#define BUSH_SIZE 500
#define WAVE_MACRO_AMPLITUDE 500
#define WAVE_MICRO_AMPLITUDE 25

#define LEFT_SKETCH_BORDER 0.063
#define RIGHT_SKETCH_BORDER 0.482
#define TOP_SKETCH_BORDER 0.13
#define BOTTOM_SKETCH_BORDER 0.87

#define STARTING_Y_OFFSET 600
#define STARTING_Z_OFFSET 5

#define STARTING_TIME 6

// QuadTree macros
#define CHUNK_SIZE 25
#define FOV_ANGLE 90

// Camera macros
#define LOS_DISTANCE 2
#define CULLING_ANGLE 45

// Sound manager macros
#define MUSIC 1
#define UI 2
#define WATER_EFFECT 3
#define WIND_EFFECT 4

#define MENU_SOUND 0
#define WORLD_SOUND 1
#define CLICK_SOUND 2
#define POP_SOUND 3
#define RESET_SOUND 4
#define SUCCESS_SOUND 5
#define WATER_SOUND 6
#define WIND_SOUND 7