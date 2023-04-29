#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <string.h>
#include "SOIL/SOIL.h"
#include "terrain.h"

using namespace std;


Terrain::Terrain()
{
    this->world_scale = 1.0f;
    this->loadFromFile("1.png");
}

Terrain::Terrain(const char* filename, float worldScale)
{
    this->world_scale = worldScale;
    this->loadFromFile(filename);
}

void Terrain::loadFromFile(const char* filename)
{
    // Load the image file using soil library
    int width, height, channels;
    unsigned char* image = SOIL_load_image(filename, &width, &height, &channels, SOIL_LOAD_AUTO);
    
    // Assert image is square
    assert(width == height);
    this->dim = width;

    // Check for an error during the load process
    assert (image != nullptr);
    
    // Allocate memory for the height map
    this->map = new Vertex[dim * dim];
    
    // Fill in the height map 
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            // Set the vertices
            this->map[i * dim + j].x = (i - dim / 2) * world_scale;
            this->map[i * dim + j].y = image[(i * dim + j)]*2;
            this->map[i * dim + j].z = (j - dim / 2) * world_scale;
        }
    }
}

// Print terrain info
void Terrain::getInfo()
{
    float min_x = 0.0f, max_x = 0.0f;
    float min_y = 0.0f, max_y = 0.0f;
    float min_z = 0.0f, max_z = 0.0f;
    
    // Find the min and max values for x,y,z
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            
            if (map[i * dim + j].x < min_x)
                min_x = map[i * dim + j].x;
            if (map[i * dim + j].x > max_x)
                max_x = map[i * dim + j].x;
            
            if (map[i * dim + j].y < min_y)
                min_y = map[i * dim + j].y;
            if (map[i * dim + j].y > max_y)
                max_y = map[i * dim + j].y;
            
            if (map[i * dim + j].z < min_z)
                min_z = map[i * dim + j].z;
            if (map[i * dim + j].z > max_z)
                max_z = map[i * dim + j].z;
        }
    }
    
    printf("Map size: {%d}x{%d}\n", dim, dim);
    printf("Min x: %f, Max x: %f\n", min_x, max_x);
    printf("Min y: %f, Max y: %f\n", min_y, max_y);
    printf("Min z: %f, Max z: %f\n", min_z, max_z);

    fflush(stdout);
}

