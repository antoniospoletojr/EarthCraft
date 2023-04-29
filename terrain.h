#ifndef TERRAIN_H
#define TERRAIN_H

#include "stdio.h"

typedef struct
{
	float x, y, z;
} Vertex;

class Terrain
{
public:
	Terrain();
	
	Terrain(const char *filename, float worldScale);

	~Terrain() { delete[] map; }

	void loadFromFile(const char *filename);
	
	// Getter for the height map
	Vertex* getMap() const { return map;}
	
	// Getter of the dimension of the height map
	int getDim() const { return dim; }
	
	// Print the min and max values for x,y,z
	void getInfo();

private:
	int dim;
	float world_scale;
	Vertex *map;
	
};



#endif
