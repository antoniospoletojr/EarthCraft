/**
@file
@brief Terrain source file.
*/

#include "Terrain.h"


// Default constructor
Terrain::Terrain()
{
    // Initialize the texture tiles
    this->tiles[0].region = HeightRegion{0.0f, 0.20f, 0.30f};
    this->tiles[1].region = HeightRegion{0.15f, 0.35f, 0.45f};
    this->tiles[2].region = HeightRegion{0.30f, 0.50f, 0.65f};
    this->tiles[3].region = HeightRegion{0.50f, 0.70f, 0.80f};
    this->tiles[4].region = HeightRegion{0.70f, 0.85f, 0.90f};
    this->tiles[5].region = HeightRegion{0.85f, 0.95f, 1.0f};
    
    this->tiles[0].texture = cv::imread("assets/textures/1.jpg", cv::IMREAD_COLOR);
    this->tiles[1].texture = cv::imread("assets/textures/2.jpg", cv::IMREAD_COLOR);
    this->tiles[2].texture = cv::imread("assets/textures/3.jpg", cv::IMREAD_COLOR);
    this->tiles[3].texture = cv::imread("assets/textures/4.jpg", cv::IMREAD_COLOR);
    this->tiles[4].texture = cv::imread("assets/textures/5.jpg", cv::IMREAD_COLOR);
    this->tiles[5].texture = cv::imread("assets/textures/6.jpg", cv::IMREAD_COLOR);
}

// Destructor
Terrain::~Terrain()
{
    delete[] heightmap;
}

// Parametrized constructor
void Terrain::initialize(float world_scale, float texture_scale)
{
    this->world_scale = world_scale;
    this->texture_scale = texture_scale;
    
    loadHeightmap();
    loadTexture();
    loadWatermap();
}

void Terrain::loadHeightmap()
{
    // Load the terrain heightmap using opencv library
    cv::Mat image = cv::imread("./assets/sketches/heightmap.png", cv::IMREAD_GRAYSCALE);
    cv::GaussianBlur(image, image, cv::Size(5, 5), 0); // Adjust the kernel size (5, 5) as needed

    unsigned char* data = image.data;
    
    // Check for an error during the load process
    assert(data != nullptr);
    
    // Assert image is square
    assert(image.rows == image.cols);

    // Assign one image side to the dim attribute for storing the heightmap lenght
    this->dim = image.rows;

    // Allocate memory for the height map
    this->heightmap = new Vec3<float>[this->dim * this->dim];
    
    // Initialize the bounds struct
    this->bounds.min_x = FLT_MAX;
    this->bounds.max_x = FLT_MIN;

    this->bounds.min_y = FLT_MAX;
    this->bounds.max_y = FLT_MIN;

    this->bounds.min_z = FLT_MAX;
    this->bounds.max_z = FLT_MIN;

    // Fill in the height map 
    for (int i = 0; i < this->dim; i++)
    {
        for (int j = 0; j < this->dim; j++)
        {
            // Set the vertices
            this->heightmap[i * dim + j].x = ((j - (dim / 2)) * this->world_scale);
            this->heightmap[i * dim + j].y = (data[(i * dim + j)] * this->world_scale);
            this->heightmap[i * dim + j].z = ((i - (dim / 2)) * this->world_scale);

            // Update the bounds based on the current Vec
            bounds.min_x = (heightmap[i * dim + j].x < bounds.min_x) ? heightmap[i * dim + j].x : bounds.min_x;
            bounds.max_x = (heightmap[i * dim + j].x > bounds.max_x) ? heightmap[i * dim + j].x : bounds.max_x;
            bounds.min_y = (heightmap[i * dim + j].y < bounds.min_y) ? heightmap[i * dim + j].y : bounds.min_y;
            bounds.max_y = (heightmap[i * dim + j].y > bounds.max_y) ? heightmap[i * dim + j].y : bounds.max_y;
            bounds.min_z = (heightmap[i * dim + j].z < bounds.min_z) ? heightmap[i * dim + j].z : bounds.min_z;
            bounds.max_z = (heightmap[i * dim + j].z > bounds.max_z) ? heightmap[i * dim + j].z : bounds.max_z;
        }
    }
}

void Terrain::loadWatermap()
{
    // Calcualte the water level
    // Calculate the total number of vertices in the map
    std::vector<int> heights(this->dim * this->dim);
    
    for (int i = 0; i < this->dim; i++)
    {
        for (int j = 0; j < this->dim; j++)
        {
            int height = (int) this->heightmap[i * this->dim + j].y;
            heights[i*this->dim +j] = height;
        }
    }
    
    // Sort the heights of the vertices in ascending order
    std::sort(heights.begin(), heights.end());
    
    // Find the index corresponding to the 10th percentile
    int percentile_index = static_cast<int>(this->dim * this->dim * FLOODING_FACTOR);
    
    printf("Water level: %d\n", heights[percentile_index]);
    
    // The value at the percentile index will be your water level
    this->water_level = heights[percentile_index];
    
    this->watermap = new Vec3<float>[this->dim * this->dim];
    
    // Fill in the height map
    for (int i = 0; i < this->dim; i++)
    {
        for (int j = 0; j < this->dim; j++)
        {
            // Set the vertices
            this->watermap[i * this->dim + j].x = ((j - (this->dim / 2)) * this->world_scale);
            this->watermap[i * this->dim + j].y = this->water_level;
            this->watermap[i * this->dim + j].z = ((i - (this->dim / 2)) * this->world_scale);
        }
    }
}

void Terrain::loadTexture()
{    
    short original_texture_size = tiles[0].texture.rows;
    // Allocate memory for the opencv texture map based on texture_1 size
    this->texture.create(original_texture_size * texture_scale, original_texture_size * texture_scale, CV_8UC3);
    float terrain_texture_ratio = (float)(this->dim / (float) texture.rows);
    
    int i_map;
    int j_map;
    float normalized_height;
    float weights[6];
    
    // Cycle through the texel which must be generated
    for (int i = 0; i < this->texture.cols; i++)
    {
        for (int j = 0; j < this->texture.rows; j++)
        {
            i_map = (int)floor(i * terrain_texture_ratio);
            j_map = (int)floor(j * terrain_texture_ratio);
            normalized_height = (float)(this->heightmap[j_map * this->dim + i_map].y / this->bounds.max_y);
            // For each of the 6 tiles, calculate the weight based on the normalized height
            for (short k = 0; k < 6; k++)
            {
                if (normalized_height >= this->tiles[k].region.low && normalized_height <= this->tiles[k].region.high)
                {
                    if (normalized_height == this->tiles[k].region.optimal)
                        weights[k] = 1;
                    else if (normalized_height < this->tiles[k].region.optimal)
                    {
                        if (k == 0)
                            weights[k] = 1;
                        else
                        {
                            float actual_diff = float(normalized_height - tiles[k].region.low);
                            float max_diff = float(tiles[k].region.optimal - tiles[k].region.low);
                            weights[k] = actual_diff / max_diff;
                        }
                    }
                    else if (normalized_height > this->tiles[k].region.optimal)
                    {
                        if (k == 5)
                            weights[k] = 1;
                        else
                        {
                            float max_diff = float(tiles[k].region.high - tiles[k].region.optimal);
                            float actual_diff = max_diff - float(normalized_height - tiles[k].region.optimal);
                            weights[k] = actual_diff / max_diff;
                        }
                    }
                }
                else
                    weights[k] = 0;
            }
            // Interpolate the pixel
            float sum = 0.0;
            for (int k = 0; k < 6; k++)
                sum += weights[k];

            cv::Vec3b interpolated_pixel(0, 0, 0);
            for (int k = 0; k < 6; k++)
                this->texture.at<cv::Vec3b>(i, j) += weights[k] / sum * tiles[k].texture.at<cv::Vec3b>(i % original_texture_size, j % original_texture_size);
        }
    }
    // Write texture to file
    // cv::imwrite("./assets/terrain_texture.png", texture);
}

// Return the height map
Vec3<float> *Terrain::getHeightmap()
{
    return heightmap;
}

// Return the water map
Vec3<float> *Terrain::getWatermap()
{
    return watermap;
}

int Terrain::getWaterLevel()
{
    return this->water_level;
}

// Return the texture map
cv::Mat Terrain::getTexture()
{
    return texture;
}

// Return the dimension of the height map
int Terrain::getDim()
{
    return dim;
}

// Return the world scale
float Terrain::getWorldDim()
{
    return dim * world_scale;
}

TerrainBounds* Terrain::getBounds()
{
    return &this->bounds;
}

// Print terrain info
void Terrain::getInfo()
{
    printf(COLOR_YELLOW);
    printf("__________________________________________\n");
    printf("Heightmap size: {%d}x{%d}\n", this->dim, this->dim);
    printf("World scale: %f\n", this->world_scale);
    printf("World dim: %f\n", this->dim * this->world_scale);
    printf("Height scale: %f\n", 10 + log10(this->world_scale));
    printf("Texture scale: %f\n", this->texture_scale);
    printf("Texture size: {%d}x{%d}\n", this->texture.rows, this->texture.cols);
    printf("Min x: %f, Max x: %f\n", this->bounds.min_x, this->bounds.max_x);
    printf("Min y: %f, Max y: %f\n", this->bounds.min_y, this->bounds.max_y);
    printf("Min z: %f, Max z: %f\n", this->bounds.min_z, this->bounds.max_z);
    printf("__________________________________________\n");
    printf(COLOR_RESET);
    
    fflush(stdout);
}

bool Terrain::checkCollision(Vec3<float> position)
{
    // Get the i,j coordinates of the height map
    int i = ((static_cast<int>(std::floor(position.z / this->world_scale + this->dim / 2)) % this->dim) + this->dim) % this->dim;
    int j = ((static_cast<int>(std::floor(position.x / this->world_scale + this->dim / 2)) % this->dim) + this->dim) % this->dim;
    
    // Get the height of the terrain at the i,j coordinates
    float height = this->heightmap[i * this->dim + j].y;
    
    // Check if the height of the terrain is greater than the height of the object (add an offset for visual purposes)
    if (height > position.y-50)
        return true;
    
    return false;
}

float Terrain::distanceFromWater(Vec3<float> position)
{
    // Get the i,j coordinates of the height map
    int i = ((static_cast<int>(std::floor(position.z / this->world_scale + this->dim / 2)) % this->dim) + this->dim) % this->dim;
    int j = ((static_cast<int>(std::floor(position.x / this->world_scale + this->dim / 2)) % this->dim) + this->dim) % this->dim;
    
    // Get the height of the terrain at the i,j coordinates
    float distance = position.y - this->watermap[i * this->dim + j].y;
    return distance;
}