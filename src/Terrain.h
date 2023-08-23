/**
@file
@brief Terrain header file.
*/

#ifndef TERRAIN_H
#define TERRAIN_H

#include "Vec.hpp"
#include "Constants.h"
#include "Colors.h"
#include <cmath>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

/**
 * @brief Struct defining the boundaries of the terrain.
 */
typedef struct
{
	float min_x;
	float max_x;
	float min_y;
	float max_y;
	float min_z;
	float max_z;
} TerrainBounds;

/**
 * @brief Struct defining the height regions of the terrain, used for texture interpolation.
 */
typedef struct
{
	float low;
	float optimal;
	float high;
} HeightRegion;

/**
 * @brief Struct defining a tile of the terrain's texture.
 */
typedef struct
{
	cv::Mat texture;
	HeightRegion region;

} TextureTile;

/**
 * @brief Terrain class which handles the terrain and water mesh generation.
 */
class Terrain
{
public:
	/**
	 * @brief Construct a new Terrain object.
	 */
	Terrain();

	/**
	 * @brief Destroy the Terrain object.
	 */
	~Terrain();
	
	/**
	 * @brief Initialize the terrain.
	 * 
	 * @param world_scale World scale factor of the terrain.
	 * @param texture_scale Texture scale factor of the terrain.
	 */
	void initialize(float world_scale, float texture_scale);

	/**
	 * @brief Get the Heightmap object.
	 * 
	 * @return Vec3<float>* 
	 */
	Vec3<float>* getHeightmap();

	/**
	 * @brief Get the Watermap object.
	 * 
	 * @return Vec3<float>* 
	 */
	Vec3<float>* getWatermap();

	/**
	 * @brief Get the water height level value.
	 * 
	 * @return int 
	 */
	int getWaterLevel();
	
	/**
	 * @brief Get the interpolated texture.
	 * 
	 * @return cv::Mat 
	 */
	cv::Mat getTexture();
	
	/**
	 * @brief Get the lenght of the heightmap png.
	 * 
	 * @return int 
	 */
	int getDim();

	/**
	 * @brief Get the real world lenght of the heightmap
	 * 
	 * @return float 
	 */
	float getWorldDim();

	/**
	 * @brief Get the Terrain Bounds object.
	 * 
	 * @return TerrainBounds* 
	 */
	TerrainBounds* getBounds();

	/**
	 * @brief Print info about the terrain.
	 */
	void getInfo();

	/**
	 * @brief Checker wether or not a position collides with the terrain.
	 * 
	 * @param position World position of the camera to check collision with
	 * @return true 
	 * @return false 
	 */
	bool checkCollision(Vec3<float> position);
	
	/**
	 * @brief Get the distance of a position from the water level.
	 * 
	 * @param position Position to check the distance from
	 * @return float 
	 */
	float distanceFromWater(Vec3<float> position);

private:
	int dim;								///< Lenght of the heightmap
	float world_scale;						///< World scale factor
	float texture_scale;					///< Texture scale factor
	TerrainBounds bounds;					///< Terrain boundaries
	int water_level;						///< Water level

	Vec3<float> *heightmap;					///< Heightmap reference
	Vec3<float> *watermap;					///< Watermap reference
	cv::Mat texture;						///< OpenCV terrain texture
	TextureTile tiles[6];					///< Array of texture tiles used for interpolation
	
	/**
	 * @brief Generate the 3D heightmap from the png file.
	 */
	void loadHeightmap();

	/**
	 * @brief Generate the watermap at the water level, which is calculated as a constant percentile of the terrain heightmap. 
	 */
	void loadWatermap();

	/**
	 * @brief Generate the terrain texture by interpolating the tiles based on the different terrain heights.
	 */
	void loadTexture();
};

#endif
