#ifndef TERRAIN_H
#define TERRAIN_H

#include "Vec3.hpp"

class Terrain
{
public:
	Terrain();
	~Terrain();
	
	void initialize(const char *filename, float worldScale);
	void loadFromFile(const char *filename); 
	Vec3<float>* getMap();
	int getDim();
	float getWorldScale();
	float getMaxHeight();
	void getInfo();

private:
	int dim;
	float world_scale;
	Vec3<float> *map;
};

#endif
