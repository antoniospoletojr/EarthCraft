#ifndef TERRAIN_H
#define TERRAIN_H

#include "vec3.hpp"

class Terrain
{
public:
	Terrain();
	Terrain(const char *filename, float worldScale);
	~Terrain();
	
	void loadFromFile(const char *filename); 
	Vec3<float>* getMap() const { return map;}
	int getDim() const { return dim; }
	void getInfo();

private:
	int dim;
	float world_scale;
	Vec3<float> *map;
};



#endif
