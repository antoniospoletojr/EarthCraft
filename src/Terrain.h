#ifndef TERRAIN_H
#define TERRAIN_H

#include "Vertex.hpp"
#include "Colors.h"
#include <cmath>
#include <opencv2/opencv.hpp>
#include <vector>

typedef struct
{
	float min_x;
	float max_x;
	float min_y;
	float max_y;
	float min_z;
	float max_z;
} TerrainBounds;

typedef struct
{
	float low;
	float optimal;
	float high;
} HeightRegion;

typedef struct
{
	cv::Mat texture;
	HeightRegion region;

} TextureTile;

class Terrain
{
public:
	Terrain();
	~Terrain();
	
	void initialize(float worldScale);
	Vertex3d<float>* getMap();
	cv::Mat getTexture();
	
	int getDim();
	float getWorldDim();
	float getMaxHeight();
	void getInfo();

private:
	int dim;
	float world_scale;
	TerrainBounds bounds;

	Vertex3d<float> *map;
	cv::Mat texture;
	TextureTile tiles[5];

	void loadMap();
	void loadTexture(float scaling);
};

#endif
