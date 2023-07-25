#ifndef TERRAIN_H
#define TERRAIN_H

#include "Vec.hpp"
#include "Constants.h"
#include "Colors.h"
#include <cmath>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

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

	void initialize(float world_scale, float texture_scale);
	Vec3<float>* getHeightmap();
	Vec3<float>* getWatermap();

	cv::Mat getTexture();
	
	int getDim();
	float getWorldDim();
	TerrainBounds* getBounds();
	int getWaterLevel();
	void getInfo();
	bool checkCollision(Vec3<float> position);

private:
	int dim;
	float world_scale, texture_scale;
	TerrainBounds bounds;

	Vec3<float> *heightmap;
	cv::Mat texture;
	TextureTile tiles[5];

	void loadHeightmap();
	void loadWatermap();
	void loadTexture();
};

#endif
