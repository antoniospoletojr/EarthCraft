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
	
	// Getter for the height map
	Vec3<float>* getMap() const { return map;}
	
	// Getter of the dimension of the height map
	int getDim() const { return dim; }
	
	// Print the min and max values for x,y,z
	void getInfo();

private:
	int dim;
	float world_scale;
	Vec3<float> *map;
};



#endif
