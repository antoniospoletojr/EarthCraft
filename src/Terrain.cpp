#include "Terrain.h"

// Default constructor
Terrain::Terrain(){}

// Destructor
Terrain::~Terrain()
{
    delete[] map;
}

// Parametrized constructor
void Terrain::initialize(float worldScale)
{
    this->world_scale = worldScale;
    loadMap();
    loadTexture(16);
}

void Terrain::loadMap()
{
    // Load the terrain heightmap using opencv library
    cv::Mat image = cv::imread("./assets/sketches/heightmap.png", cv::IMREAD_GRAYSCALE);
    unsigned char* data = image.data;
    
    // Check for an error during the load process
    assert(data != nullptr);
    
    // Assert image is square
    assert(image.rows == image.cols);

    // Assign one image side to the dim attribute for storing the heightmap lenght
    this->dim = image.rows;

    // Allocate memory for the height map
    this->map = new Vertex3d<float>[this->dim * this->dim];
    
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
            this->map[i * dim + j].x = ((i - dim / 2) * this->world_scale);
            this->map[i * dim + j].y = (data[(i * dim + j)] * (15 + log(this->world_scale)));
            this->map[i * dim + j].z = ((j - dim / 2) * this->world_scale);

            // Update the bounds based on the current vertex
            bounds.min_x = (map[i * dim + j].x < bounds.min_x) ? map[i * dim + j].x : bounds.min_x;
            bounds.max_x = (map[i * dim + j].x > bounds.max_x) ? map[i * dim + j].x : bounds.max_x;
            bounds.min_y = (map[i * dim + j].y < bounds.min_y) ? map[i * dim + j].y : bounds.min_y;
            bounds.max_y = (map[i * dim + j].y > bounds.max_y) ? map[i * dim + j].y : bounds.max_y;
            bounds.min_z = (map[i * dim + j].z < bounds.min_z) ? map[i * dim + j].z : bounds.min_z;
            bounds.max_z = (map[i * dim + j].z > bounds.max_z) ? map[i * dim + j].z : bounds.max_z;
        }
    }
}

void Terrain::loadTexture(float scaling)
{
    // Load terrain texture tiles using opencv library
    cv::Mat texture_1 = cv::imread("assets/textures/1.jpg", cv::IMREAD_COLOR);
    cv::Mat texture_2 = cv::imread("assets/textures/2.jpg", cv::IMREAD_COLOR);
    cv::Mat texture_3 = cv::imread("assets/textures/3.jpg", cv::IMREAD_COLOR);
    cv::Mat texture_4 = cv::imread("assets/textures/4.jpg", cv::IMREAD_COLOR);
    cv::Mat texture_5 = cv::imread("assets/textures/5.jpg", cv::IMREAD_COLOR);

    // Check for an error during the load process
    assert(texture_1.data != nullptr);
    assert(texture_2.data != nullptr);
    assert(texture_3.data != nullptr);
    assert(texture_4.data != nullptr);
    assert(texture_5.data != nullptr);
    
    short original_texture_size = texture_1.rows;
    // Allocate memory for the opencv texture map based on texture_1 size
    this->texture.create(original_texture_size * scaling, original_texture_size * scaling, CV_8UC3);
    float terrain_texture_ratio = (float)this->dim / (float) texture.rows;
    
    printf("Texture size: {%d}x{%d}\n", original_texture_size * scaling, original_texture_size * scaling);
    printf("Terrain texture ratio: %f\n", terrain_texture_ratio);
    
    int i_map;
    int j_map;
    float normalized_height;

    // Cycle through the texture which must be generated
    for (int i = 0; i < this->texture.cols; i++)
    {
        for (int j = 0; j < this->texture.rows; j++)
        {
            i_map = (int)floor(i * terrain_texture_ratio);
            j_map = (int)floor(j * terrain_texture_ratio);
            normalized_height = this->map[j_map * dim + i_map].y / this->bounds.max_y;
            
            // if height/this->bounds.max_y < 0.2 then assign texture_1, and similarly check for the other textures
            if (normalized_height < 0.15) // 0 - 15 - 25
            {
                this->texture.at<cv::Vec3b>(i, j) = texture_1.at<cv::Vec3b>(i % original_texture_size, j % original_texture_size);
            }
            else if (normalized_height < 0.3) // 15 - 30 - 50
            {   
                this->texture.at<cv::Vec3b>(i, j) = texture_2.at<cv::Vec3b>(i % original_texture_size, j % original_texture_size);
            }
            else if (normalized_height < 0.5) // 30 - 50 - 70
            {
                this->texture.at<cv::Vec3b>(i, j) = texture_3.at<cv::Vec3b>(i % original_texture_size, j % original_texture_size);
            }
            else if (normalized_height < 0.75) // 50 - 75 - 95
            {
                this->texture.at<cv::Vec3b>(i, j) = texture_4.at<cv::Vec3b>(i % original_texture_size, j % original_texture_size);
            }
            else // 90 - 100
            {
                this->texture.at<cv::Vec3b>(i, j) = texture_5.at<cv::Vec3b>(i % original_texture_size, j % original_texture_size);
            }
        }
    }
}

// Return the height map
Vertex3d<float> *Terrain::getMap()
{
    return map;
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

float Terrain::getMaxHeight()
{
    return this->bounds.max_y;
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
    printf("Min x: %f, Max x: %f\n", this->bounds.min_x, this->bounds.max_x);
    printf("Min y: %f, Max y: %f\n", this->bounds.min_y, this->bounds.max_y);
    printf("Min z: %f, Max z: %f\n", this->bounds.min_z, this->bounds.max_z);
    printf("__________________________________________\n");
    printf(COLOR_RESET);

    fflush(stdout);
}