#include <stdio.h>
#include <assert.h>
#include <cmath>
#include "stb_image/stb_image.h"
#include "Terrain.h"

// Default constructor
Terrain::Terrain(){}

// Destructor
Terrain::~Terrain()
{
    delete[] map;
}

// Parametrized constructor
void Terrain::initialize(const char* filename, float worldScale)
{
    this->world_scale = worldScale;
    loadFromFile(filename);
}

void Terrain::loadFromFile(const char* filename)
{
    // Load the image file using soil library
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    
    // Assert image is square
    assert(width == height);
    dim = width;
    
    // Check for an error during the load process
    assert (image != nullptr);
    
    // Allocate memory for the height map
    map = new Vec3<float>[dim * dim];
    
    // Fill in the height map 
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            // Set the vertices
            map[i * dim + j].set_x((i - dim / 2) * world_scale);
            map[i * dim + j].set_y(image[(i * dim + j)] * (10 + log(world_scale)));
            map[i * dim + j].set_z((j - dim / 2) * world_scale);
        }
    }
}

// Return the height map
Vec3<float>* Terrain::getMap(){return map;}

// Return the dimension of the height map
int Terrain::getDim(){return dim;}

// Return the world scale
float Terrain::getWorldDim(){return dim*world_scale;}

float Terrain::getMaxHeight()
{
    float max_height = 0.0f;
    
    // Find the max height
    for (int i = 0; i < dim * dim; i++)
    {
        if (map[i].y() > max_height)
            max_height = map[i].y();
    }
    
    return max_height;
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
            
            if (map[i * dim + j].x() < min_x)
                min_x = map[i * dim + j].x();
            if (map[i * dim + j].x() > max_x)
                max_x = map[i * dim + j].x();
            
            if (map[i * dim + j].y() < min_y)
                min_y = map[i * dim + j].y();
            if (map[i * dim + j].y() > max_y)
                max_y = map[i * dim + j].y();
            
            if (map[i * dim + j].z() < min_z)
                min_z = map[i * dim + j].z();
            if (map[i * dim + j].z() > max_z)
                max_z = map[i * dim + j].z();
        }
    }
    
    printf("__________________________________________\n");
    printf("Heightmap size: {%d}x{%d}\n", dim, dim);
    printf("World scale: %f\n", world_scale);
    printf("World dim: %f\n", dim*world_scale);
    printf("Height scale: %f\n", 1 + log10(world_scale));
    printf("Min x: %f, Max x: %f\n", min_x, max_x);
    printf("Min y: %f, Max y: %f\n", min_y, max_y);
    printf("Min z: %f, Max z: %f\n", min_z, max_z);
    printf("__________________________________________\n");

    fflush(stdout);
}