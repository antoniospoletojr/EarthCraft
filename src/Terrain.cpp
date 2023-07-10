#include "Terrain.h"

// Default constructor
Terrain::Terrain()
{
    // Initialize the texture tiles
    this->tiles[0].region = HeightRegion{0.f, 0.15f, 0.25f};
    this->tiles[1].region = HeightRegion{0.15f, 0.30f, 0.50f};
    this->tiles[2].region = HeightRegion{0.30f, 0.50f, 0.70f};
    this->tiles[3].region = HeightRegion{0.50f, 0.75f, 0.95f};
    this->tiles[4].region = HeightRegion{0.90f, 0.95f, 1.0f};
    
    this->tiles[0].texture = cv::imread("assets/textures/1.jpg", cv::IMREAD_COLOR);
    this->tiles[1].texture = cv::imread("assets/textures/2.jpg", cv::IMREAD_COLOR);
    this->tiles[2].texture = cv::imread("assets/textures/3.jpg", cv::IMREAD_COLOR);
    this->tiles[3].texture = cv::imread("assets/textures/4.jpg", cv::IMREAD_COLOR);
    this->tiles[4].texture = cv::imread("assets/textures/5.jpg", cv::IMREAD_COLOR);
}

// Destructor
Terrain::~Terrain()
{
    delete[] map;
}

// Parametrized constructor
void Terrain::initialize(float world_scale, float texture_scale)
{
    this->world_scale = world_scale;
    this->texture_scale = texture_scale;
    
    loadMap();
    loadTexture();
}

void Terrain::loadMap()
{
    // Load the terrain heightmap using opencv library
    cv::Mat image = cv::imread("./assets/sketches/heightmap.png", cv::IMREAD_GRAYSCALE);
    //cv::Mat smoothedImage;
    cv::GaussianBlur(image, image, cv::Size(5, 5), 0); // Adjust the kernel size (5, 5) as needed
    
    unsigned char* data = image.data;
    
    // Check for an error during the load process
    assert(data != nullptr);
    
    // Assert image is square
    assert(image.rows == image.cols);

    // Assign one image side to the dim attribute for storing the heightmap lenght
    this->dim = image.rows;

    // Allocate memory for the height map
    this->map = new Vec3<float>[this->dim * this->dim];
    
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
            this->map[i * dim + j].y = (data[(j * dim + i)] * (15 + log(this->world_scale)));
            this->map[i * dim + j].z = ((j - dim / 2) * this->world_scale);
            
            // Update the bounds based on the current Vec
            bounds.min_x = (map[i * dim + j].x < bounds.min_x) ? map[i * dim + j].x : bounds.min_x;
            bounds.max_x = (map[i * dim + j].x > bounds.max_x) ? map[i * dim + j].x : bounds.max_x;
            bounds.min_y = (map[i * dim + j].y < bounds.min_y) ? map[i * dim + j].y : bounds.min_y;
            bounds.max_y = (map[i * dim + j].y > bounds.max_y) ? map[i * dim + j].y : bounds.max_y;
            bounds.min_z = (map[i * dim + j].z < bounds.min_z) ? map[i * dim + j].z : bounds.min_z;
            bounds.max_z = (map[i * dim + j].z > bounds.max_z) ? map[i * dim + j].z : bounds.max_z;
        }
    }
}

void Terrain::loadTexture()
{    
    short original_texture_size = tiles[0].texture.rows;
    // Allocate memory for the opencv texture map based on texture_1 size
    this->texture.create(original_texture_size * texture_scale, original_texture_size * texture_scale, CV_8UC3);
    float terrain_texture_ratio = (float)this->dim / (float) texture.rows;
    
    int i_map;
    int j_map;
    float normalized_height;
    float weights[5];

    // Cycle through the texture which must be generated
    for (int i = 0; i < this->texture.cols; i++)
    {
        for (int j = 0; j < this->texture.rows; j++)
        {
            i_map = (int)floor(i * terrain_texture_ratio);
            j_map = (int)floor(j * terrain_texture_ratio);
            normalized_height = (float) (this->map[j_map * dim + i_map].y / this->bounds.max_y);
            
            for (short k = 0; k < 5; k++)
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
                        if (k == 4)
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
            
            
            float sum = 0.0;
            for (int k = 0; k < 5; k++)
                sum += weights[k];

            cv::Vec3b interpolated_pixel(0, 0, 0);
            for (int k = 0; k < 5; k++)
                this->texture.at<cv::Vec3b>(i, j) += weights[k] / sum * tiles[k].texture.at<cv::Vec3b>(i % original_texture_size, j % original_texture_size);
        }
    }
    // Write texture to file
    //cv::imwrite("./assets/terrain_texture.png", texture);
}

// Return the height map
Vec3<float> *Terrain::getMap()
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
    printf("Texture scale: %f\n", this->texture_scale);
    printf("World dim: %f\n", this->dim * this->world_scale);
    printf("Height scale: %f\n", 10 + log10(this->world_scale));
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
    int i = ((static_cast<int>(std::floor(position.x / this->world_scale + this->dim / 2)) % this->dim) + this->dim) % this->dim;
    int j = ((static_cast<int>(std::floor(position.z / this->world_scale + this->dim / 2)) % this->dim) + this->dim) % this->dim;
    
    // Get the height of the terrain at the i,j coordinates
    float height = this->map[i * this->dim + j].y;

    // Check if the height of the terrain is greater than the height of the object (add an offset for visual purposes)
    if (height > position.y-50)
        return true;
    
    return false;
}