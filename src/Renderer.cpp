/**
@file
@brief Renderer source file.
*/

#include "Renderer.h"

using namespace std;


Renderer *Renderer::instance = nullptr;

// Default constructor
Renderer::Renderer()
{
    if (Renderer::instance == nullptr)
        Renderer::instance = this;
}

// Destructor
Renderer::~Renderer()
{
    // Disable the vertex arrays
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    
    // Unbind any buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // Deallocate memory
    objects.clear();
    
    // Delete the vertex array objects
    glDeleteVertexArrays(1, &objects[SUN].vao);
    glDeleteVertexArrays(1, &objects[MOON].vao);
    glDeleteVertexArrays(1, &objects[SPLASHSCREEN].vao);
    glDeleteVertexArrays(1, &objects[CANVAS].vao);
    glDeleteVertexArrays(1, &objects[SKYDOME].vao);
    glDeleteVertexArrays(1, &objects[WATER].vao);
    glDeleteVertexArrays(1, &objects[VEGETATION].vao);
    
    // Deallocate opencv objects
    menu_clips[LANDING_SCREEN].release();
    menu_clips[RIDGES_SCREEN].release();
    menu_clips[PEAKS_SCREEN].release();
    menu_clips[RIVERS_SCREEN].release();
    menu_clips[BASINS_SCREEN].release();
    menu_clips[LOADING_SCREEN].release();
    menu_frame.release();

    Renderer::instance = nullptr;
}

void Renderer::setTerrain(Terrain *terrain)
{
    this->terrain = terrain;
    // Print the map info
    this->terrain->getInfo();
}

void Renderer::initialize(Camera *camera, QuadTree *quadtree)
{
    // Set the camera
    this->camera = camera;

    // Set the quadtree object
    this->quadtree = quadtree;
    
    // Allocate space for 8 objects (Splashscreen, Canvas, Skydome, Sun, Moon, Water, Vegetation and the 4 sketches)
    objects.resize(11);
    
    // Initialize non-terrain-related objects
    this->initializeSplashscreen();
    this->initializeCanvas();
    this->initializeSkydome();
    
    // Set the glut timer callback for the sun animaton
    glutTimerFunc(100, Renderer::timerCallback, 0);
    
    // Set the glut display callback
    glutDisplayFunc(Renderer::draw);
    
    const siv::PerlinNoise::seed_type seed = 12345;
    this->perlin_noise = siv::PerlinNoise(seed);
}

void Renderer::initializeWater()
{
    // Retrieve the map
    Vec3<float> *map = this->terrain->getWatermap();
    
    int dim = this->terrain->getDim();
    
    objects[WATER].vertices.clear();
    objects[WATER].indices.clear();
    objects[WATER].textures.clear();
    objects[WATER].normals.clear();
    
    // Find depression points by comparing each low point with the neighbors
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            objects[WATER].vertices.push_back(map[i * dim + j].x);
            objects[WATER].vertices.push_back(map[i * dim + j].y);
            objects[WATER].vertices.push_back(map[i * dim + j].z);
            
            objects[WATER].textures.push_back((float)i / dim * 100);
            objects[WATER].textures.push_back((float)j / dim * 100);
            
            objects[WATER].normals.push_back(0.0f);
            objects[WATER].normals.push_back(0.0f);
            objects[WATER].normals.push_back(0.0f);
        }
    }
    
    // Generate indices for triangle strips
    for (int z = 0; z < dim - 1; z++) // 449
    {
        // Start a new strip
        objects[WATER].indices.push_back(z * dim);
        for (int x = 0; x < dim; x++) // 902
        {
            // Add vertices to strip
            objects[WATER].indices.push_back((z + 1) * dim + x);
            objects[WATER].indices.push_back(z * dim + x);
        }
        // Use primitive restart to start a new strip
        objects[WATER].indices.push_back(0xFFFFFFFFu);
    }
    
    // Generate and bind a texture object
    glGenTextures(1, &objects[WATER].texture[0]);
    glBindTexture(GL_TEXTURE_2D, objects[WATER].texture[0]);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    cv::Mat water_texture = cv::imread("./assets/textures/water.jpg");
    cv::cvtColor(water_texture, water_texture, cv::COLOR_BGR2BGRA);

    // Set the alpha channel to 0.5
    for (int i = 0; i < water_texture.rows; i++)
    {
        for (int j = 0; j < water_texture.cols; j++)
        {
            water_texture.at<cv::Vec4b>(i, j)[3] = 128;
        }
    }

    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, water_texture.cols, water_texture.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, water_texture.data);

    // Generate the vertex array object for the mesh
    glGenVertexArrays(1, &objects[WATER].vao);
    // Bind the vertex array object for the mesh
    glBindVertexArray(objects[WATER].vao);
    
    // Generate the buffer objects
    glGenBuffers(1, &objects[WATER].vbo);
    glGenBuffers(1, &objects[WATER].tbo);
    glGenBuffers(1, &objects[WATER].ibo);
    glGenBuffers(1, &objects[WATER].nbo);

    // Bind and fill the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[WATER].vbo);
    glBufferData(GL_ARRAY_BUFFER, objects[WATER].vertices.size() * sizeof(float), objects[WATER].vertices.data(), GL_STREAM_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[WATER].tbo);
    glBufferData(GL_ARRAY_BUFFER, objects[WATER].textures.size() * sizeof(float), objects[WATER].textures.data(), GL_STREAM_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
    
    // Bind and fill the normals buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[WATER].nbo);
    glBufferData(GL_ARRAY_BUFFER, objects[WATER].normals.size() * sizeof(float), objects[WATER].normals.data(), GL_STREAM_DRAW);
    glNormalPointer(GL_FLOAT, 0, 0);
    
    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[WATER].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, objects[WATER].indices.size() * sizeof(GLuint), objects[WATER].indices.data(), GL_STATIC_DRAW);

    // Unbind everything
    glBindVertexArray(0);
}

void Renderer::initializeVegetation()
{
    // Retrieve the map
    Vec3<float> *map = this->terrain->getHeightmap();
    int water_level = this->terrain->getWaterLevel();

    int dim = this->terrain->getDim();
    
    objects[VEGETATION].vertices.clear();
    objects[VEGETATION].textures.clear();
    
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            if (rand() % VEGETATION_SPARSITY == 0)
            {
                if (map[i * dim + j].y > water_level + WAVE_MACRO_AMPLITUDE)
                {
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].x + BUSH_SIZE);
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].y);
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].z);
                    
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].x + BUSH_SIZE);
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].y + BUSH_SIZE);
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].z);
                    
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].x - BUSH_SIZE);
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].y + BUSH_SIZE);
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].z);
                    
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].x - BUSH_SIZE);
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].y);
                    objects[VEGETATION].vertices.push_back(map[i * dim + j].z);
                    
                    if (map[i * dim + j].y < this->terrain->getBounds()->max_y * 0.4)
                    {
                        objects[VEGETATION].textures.push_back(0.5f);
                        objects[VEGETATION].textures.push_back(1.0f);
                        
                        objects[VEGETATION].textures.push_back(0.5f);
                        objects[VEGETATION].textures.push_back(0.0f);
                        
                        objects[VEGETATION].textures.push_back(0.0f);
                        objects[VEGETATION].textures.push_back(0.0f);
                        
                        objects[VEGETATION].textures.push_back(0.0f);
                        objects[VEGETATION].textures.push_back(1.0f);
                    }
                    else
                    {   
                        objects[VEGETATION].textures.push_back(1.0f);
                        objects[VEGETATION].textures.push_back(1.0f);
                        
                        objects[VEGETATION].textures.push_back(1.0f);
                        objects[VEGETATION].textures.push_back(0.0f);
                        
                        objects[VEGETATION].textures.push_back(0.5f);
                        objects[VEGETATION].textures.push_back(0.0f);
                        
                        objects[VEGETATION].textures.push_back(0.5f);
                        objects[VEGETATION].textures.push_back(1.0f);
                    }
                }
            }
        }
    }
    
    // Generate and bind a texture object
    glGenTextures(1, &objects[VEGETATION].texture[0]);
    glBindTexture(GL_TEXTURE_2D, objects[VEGETATION].texture[0]);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Load grass texture in opencv using 4 channels
    cv::Mat grass_texture = cv::imread("./assets/textures/grass.png", cv::IMREAD_UNCHANGED);

    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, grass_texture.cols, grass_texture.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, grass_texture.data);

    // Generate the vertex array object for the mesh
    glGenVertexArrays(1, &objects[VEGETATION].vao);
    // Bind the vertex array object for the mesh
    glBindVertexArray(objects[VEGETATION].vao);
    
    // Generate the buffer objects
    glGenBuffers(1, &objects[VEGETATION].vbo);
    glGenBuffers(1, &objects[VEGETATION].tbo);
    
    // Bind and fill the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[VEGETATION].vbo);
    glBufferData(GL_ARRAY_BUFFER, objects[VEGETATION].vertices.size() * sizeof(float), objects[VEGETATION].vertices.data(), GL_STREAM_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[VEGETATION].tbo);
    glBufferData(GL_ARRAY_BUFFER, objects[VEGETATION].textures.size() * sizeof(float), objects[VEGETATION].textures.data(), GL_STREAM_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Unbind everything
    glBindVertexArray(0);
}

void Renderer::initializeOrbit(int orbit_height)
{

    // SUN OBJECT
    
    // Clear the sun object before initializing it
    objects[SUN].vertices.clear();
    objects[SUN].indices.clear();
    objects[SUN].textures.clear();

    // Load sun texture image
    cv::Mat sun_texture = cv::imread("./assets/textures/sun.png");
    
    // Check if the image was loaded successfully
    if (sun_texture.empty())
    {
        // Handle error
        std::cerr << "Failed to load texture image." << std::endl;
        return;
    }

    // Generate and bind a texture object
    glGenTextures(1, &objects[SUN].texture[0]);
    glBindTexture(GL_TEXTURE_2D, objects[SUN].texture[0]);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sun_texture.cols, sun_texture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, sun_texture.data);

    Assimp::Importer importer;

    // Load the .obj file
    const aiScene *sun_scene = importer.ReadFile("./assets/models/sun.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    // Check if the scene was loaded successfully
    if (!sun_scene || sun_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !sun_scene->mRootNode)
        std::cout << "Error loading scene: " << importer.GetErrorString() << std::endl;

    // For each mesh in the scene
    for (unsigned int i = 0; i < sun_scene->mNumMeshes; ++i)
    {
        const aiMesh *mesh = sun_scene->mMeshes[i];

        // For each vertexin the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            // Extract vertices
            aiVector3D Vec = mesh->mVertices[j];
            objects[SUN].vertices.push_back(Vec.x);
            objects[SUN].vertices.push_back(Vec.y - orbit_height*2);
            objects[SUN].vertices.push_back(Vec.z);

            // Extract texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                objects[SUN].textures.push_back(texCoord.x);
                objects[SUN].textures.push_back(texCoord.y);
            }
        }

        // For each face in the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace &face = mesh->mFaces[j];

            // Assume triangular faces
            if (face.mNumIndices == 3)
            {
                objects[SUN].indices.push_back(face.mIndices[0]);
                objects[SUN].indices.push_back(face.mIndices[1]);
                objects[SUN].indices.push_back(face.mIndices[2]);
            }
        }
    }
    
    // Generate the vertex array object for the sun
    glGenVertexArrays(1, &objects[SUN].vao);
    // Bind the vertex array object for the sun
    glBindVertexArray(objects[SUN].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[SUN].vbo);
    glGenBuffers(1, &objects[SUN].tbo);
    glGenBuffers(1, &objects[SUN].ibo);

    // Bind and fill the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SUN].vbo);
    glBufferData(GL_ARRAY_BUFFER, objects[SUN].vertices.size() * sizeof(float), objects[SUN].vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SUN].tbo);
    glBufferData(GL_ARRAY_BUFFER, objects[SUN].textures.size() * sizeof(float), objects[SUN].textures.data(), GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
    
    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[SUN].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, objects[SUN].indices.size() * sizeof(GLuint), objects[SUN].indices.data(), GL_STATIC_DRAW);

    // Unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // MOON OBJECT
    
    // Clear the moon object before initializing it
    objects[MOON].vertices.clear();
    objects[MOON].indices.clear();
    objects[MOON].textures.clear();

    // Load skydome texture image
    cv::Mat moon_texture = cv::imread("./assets/textures/moon.jpg");
    
    // Check if the image was loaded successfully
    if (moon_texture.empty())
    {
        // Handle error
        std::cerr << "Failed to load texture image." << std::endl;
        return;
    }
    
    // Generate and bind a texture object
    glGenTextures(1, &objects[MOON].texture[0]);
    glBindTexture(GL_TEXTURE_2D, objects[MOON].texture[0]);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, moon_texture.cols, moon_texture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, moon_texture.data);

    // Load the .obj file
    const aiScene *moon_scene = importer.ReadFile("./assets/models/moon.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    // Check if the scene was loaded successfully
    if (!moon_scene || moon_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !moon_scene->mRootNode)
        std::cout << "Error loading scene: " << importer.GetErrorString() << std::endl;

    // For each mesh in the scene
    for (unsigned int i = 0; i < moon_scene->mNumMeshes; ++i)
    {
        const aiMesh *mesh = moon_scene->mMeshes[i];

        // For each vertexin the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            // Extract vertices
            aiVector3D Vec = mesh->mVertices[j];
            objects[MOON].vertices.push_back(Vec.x);
            objects[MOON].vertices.push_back(Vec.y + orbit_height*2);
            objects[MOON].vertices.push_back(Vec.z);
            
            // Extract texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                objects[MOON].textures.push_back(texCoord.x);
                objects[MOON].textures.push_back(texCoord.y);
            }
        }

        // For each face in the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace &face = mesh->mFaces[j];

            // Assume triangular faces
            if (face.mNumIndices == 3)
            {
                objects[MOON].indices.push_back(face.mIndices[0]);
                objects[MOON].indices.push_back(face.mIndices[1]);
                objects[MOON].indices.push_back(face.mIndices[2]);
            }
        }
    }

    // Generate the vertex array object for the sun
    glGenVertexArrays(1, &objects[MOON].vao);
    // Bind the vertex array object for the sun
    glBindVertexArray(objects[MOON].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[MOON].vbo);
    glGenBuffers(1, &objects[MOON].tbo);
    glGenBuffers(1, &objects[MOON].ibo);

    // Bind and fill the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MOON].vbo);
    glBufferData(GL_ARRAY_BUFFER, objects[MOON].vertices.size() * sizeof(float), objects[MOON].vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MOON].tbo);
    glBufferData(GL_ARRAY_BUFFER, objects[MOON].textures.size() * sizeof(float), objects[MOON].textures.data(), GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[MOON].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, objects[MOON].indices.size() * sizeof(GLuint), objects[MOON].indices.data(), GL_STATIC_DRAW);

    // Unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::initializeSkydome()
{
    // Load skydome texture image
    cv::Mat day_texture = cv::imread("./assets/textures/day.jpg");
    // Check if the image was loaded successfully
    if (day_texture.empty())
    {
        // Handle error
        std::cerr << "Failed to load skydome texture image." << std::endl;
        return;
    }
    // Add alpha channel to day_texture in place
    cv::cvtColor(day_texture, day_texture, cv::COLOR_BGR2BGRA);
    
    // Load night texture image
    cv::Mat night_texture = cv::imread("./assets/textures/night.jpg");
    // Check if the image was loaded successfully
    if (night_texture.empty())
    {
        // Handle error
        std::cerr << "Failed to load night texture image." << std::endl;
        return;
    }
    // Add alpha channel to night_texture in place
    cv::cvtColor(night_texture, night_texture, cv::COLOR_BGR2BGRA);

    // Generate and bind a texture object
    glGenTextures(1, &objects[SKYDOME].texture[0]);
    glBindTexture(GL_TEXTURE_2D, objects[SKYDOME].texture[0]);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, day_texture.cols, day_texture.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, day_texture.data);

    // Generate and bind a texture object for the night texture
    glGenTextures(1, &objects[SKYDOME].texture[1]);
    glBindTexture(GL_TEXTURE_2D, objects[SKYDOME].texture[1]);
    
    // Set texture parameters for the night texture (similar to day texture parameters)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload the night texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, night_texture.cols, night_texture.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, night_texture.data);

    // Assimp importer
    Assimp::Importer importer;

    // Load the .obj file
    const aiScene *scene = importer.ReadFile("./assets/models/skydome.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    // Check if the scene was loaded successfully
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "Error loading skydome model: " << importer.GetErrorString() << std::endl;
        return;
    }

    // For each mesh in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh *mesh = scene->mMeshes[i];
        
        // For each vertexin the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            // Extract vertices
            aiVector3D Vec = mesh->mVertices[j];
            objects[SKYDOME].vertices.push_back(Vec.x*4);
            objects[SKYDOME].vertices.push_back(Vec.y*4);
            objects[SKYDOME].vertices.push_back(Vec.z*4);

            // Extract texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                objects[SKYDOME].textures.push_back(texCoord.y);
                objects[SKYDOME].textures.push_back(texCoord.x);
            }
        }

        // For each face in the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace &face = mesh->mFaces[j];

            // Assume triangular faces
            if (face.mNumIndices == 3)
            {
                objects[SKYDOME].indices.push_back(face.mIndices[0]);
                objects[SKYDOME].indices.push_back(face.mIndices[1]);
                objects[SKYDOME].indices.push_back(face.mIndices[2]);
            }
        }
    }
    
    // Generate the vertex array object for the skydome
    glGenVertexArrays(1, &objects[SKYDOME].vao);
    
    // Bind the vertex array object for the skydome
    glBindVertexArray(objects[SKYDOME].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[SKYDOME].vbo);
    glGenBuffers(1, &objects[SKYDOME].tbo);
    glGenBuffers(1, &objects[SKYDOME].ibo);

    // Bind and fill the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SKYDOME].vbo);
    glBufferData(GL_ARRAY_BUFFER, objects[SKYDOME].vertices.size() * sizeof(float), objects[SKYDOME].vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    
    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SKYDOME].tbo);
    glBufferData(GL_ARRAY_BUFFER, objects[SKYDOME].textures.size() * sizeof(float), objects[SKYDOME].textures.data(), GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
    
    // Bind and fill the index buffer object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[SKYDOME].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, objects[SKYDOME].indices.size() * sizeof(unsigned int), objects[SKYDOME].indices.data(), GL_STATIC_DRAW);

    // Unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::initializeSplashscreen()
{
    // Load the splashscreen video and the first frame
    instance->menu_clips[LANDING_SCREEN].open("./assets/menu/Splashscreen.mp4");
    instance->menu_clips[LANDING_SCREEN].read(instance->menu_frame);

    // Generate the vertex array object for the SPLASHSCREEN
    glGenVertexArrays(1, &objects[SPLASHSCREEN].vao);
    // Bind the vertex array object for the SPLASHSCREEN
    glBindVertexArray(objects[SPLASHSCREEN].vao);
    
    // Generate the vertex buffer objects
    glGenBuffers(1, &objects[SPLASHSCREEN].vbo);
    // Generate the texture buffer objects
    glGenBuffers(1, &objects[SPLASHSCREEN].tbo);
    // Generate the texture buffer objects
    glGenBuffers(1, &objects[SPLASHSCREEN].cbo);

    // Create vertexdata for the quad
    std::vector<GLfloat> vertices(8);
    std::vector<GLfloat> colors = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    std::vector<GLfloat> texture_coords = {0, 1, 1, 1, 1, 0, 0, 0};

    // Enable the vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Bind the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SPLASHSCREEN].vbo);
    // Copy data into the vertex buffer
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    // Specify vertexpointer location
    glVertexPointer(2, GL_FLOAT, 0, 0);

    // Bind the color buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SPLASHSCREEN].cbo);
    // Copy data into the color buffer
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
    // Specify color pointer location
    glColorPointer(4, GL_FLOAT, 0, 0);

    // Bind the texture buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SPLASHSCREEN].tbo);
    // Copy data into the texture buffer
    glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(GLfloat), texture_coords.data(), GL_STATIC_DRAW);
    // Specify texture pointer location
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Unbind everything
    glBindVertexArray(0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glGenTextures(1, &objects[SPLASHSCREEN].texture[0]);
}

void Renderer::initializeCanvas()
{
    // Load assets for the canvas pages
    instance->menu_clips[RIDGES_SCREEN].open("./assets/menu/Ridges.mp4");
    instance->menu_clips[PEAKS_SCREEN].open("./assets/menu/Peaks.mp4");
    instance->menu_clips[RIVERS_SCREEN].open("./assets/menu/Rivers.mp4");
    instance->menu_clips[BASINS_SCREEN].open("./assets/menu/Basins.mp4");
    instance->menu_clips[LOADING_SCREEN].open("./assets/menu/Loadingscreen.mp4");

    // Generate the vertex array object for the canvas
    glGenVertexArrays(1, &objects[CANVAS].vao);
    // Bind the vertex array object for the canvas
    glBindVertexArray(objects[CANVAS].vao);

    // Generate the vertex buffer objects
    glGenBuffers(1, &objects[CANVAS].vbo);
    // Generate the texture buffer objects
    glGenBuffers(1, &objects[CANVAS].tbo);
    // Generate the texture buffer objects
    glGenBuffers(1, &objects[CANVAS].cbo);

    // Create vertexdata for the quad
    std::vector<GLfloat> vertices(8);
    std::vector<GLfloat> colors = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    std::vector<GLfloat> texture_coords = {0, 1, 1, 1, 1, 0, 0, 0};

    // Enable the vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Bind the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[CANVAS].vbo);
    // Copy data into the vertex buffer
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    // Specify vertexpointer location
    glVertexPointer(2, GL_FLOAT, 0, 0);

    // Bind the color buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[CANVAS].cbo);
    // Copy data into the color buffer
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
    // Specify color pointer location
    glColorPointer(4, GL_FLOAT, 0, 0);

    // Bind the texture buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[CANVAS].tbo);
    // Copy data into the texture buffer
    glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(GLfloat), texture_coords.data(), GL_STATIC_DRAW);
    // Specify texture pointer location
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Unbind everything
    glBindVertexArray(0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glGenTextures(1, &objects[CANVAS].texture[0]);
}

void Renderer::takeSnapshot()
{
    short current_canvas = current_menu_page - 1;

    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport); // Save the previous viewport dimensions [x, y, width, height]

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer); // Generate a framebuffer object
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); // Bind the framebuffer for configuration

    // Generate a texture for color rendering
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer); // Generate a texture
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer); // Bind the texture for configuration
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // Allocate texture memory
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Set texture filtering for minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Set texture filtering for magnification
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

    // Attach the texture to the currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Generate a renderbuffer for depth and stencil operations
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo); // Generate a renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); // Bind the renderbuffer for configuration
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080); // Allocate storage for the renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, 0); // Unbind the renderbuffer

    // Attach the renderbuffer to the framebuffer's depth and stencil attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Check if the framebuffer is complete and usable
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // Set up rendering parameters and draw
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); // Rebind the framebuffer for rendering
    glClearColor(1, 1, 1, 1); // Set the clear color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color and depth buffers
    glViewport(0, 0, 1920, 1080); // Set the viewport to match the framebuffer size
    instance->drawSketch(current_canvas); // Render the sketch using the specified canvas
    

    // Define the snapshot matrix
    cv::Mat snapshot(800, 800, CV_8UC3);
    // Read the pixels from the framebuffer
    glReadPixels(123, 140, 800, 800, GL_BGR, GL_UNSIGNED_BYTE, snapshot.data);
    // Flip the image vertically
    cv::flip(snapshot, snapshot, 0);
    // Convert to grayscale
    cv::cvtColor(snapshot, snapshot, cv::COLOR_BGR2GRAY);
    // Convert any non-white pixel to black
    cv::threshold(snapshot, snapshot, 254, 255, cv::THRESH_BINARY);
    // Invert the image
    cv::bitwise_not(snapshot, snapshot);
    // Rescale to 450x450
    cv::resize(snapshot, snapshot, cv::Size(450, 450), 0, 0, cv::INTER_AREA);
    // Define filename string based on current menu page
    std::string filename;
    switch (current_menu_page)
    {
    case RIDGES_SCREEN:
        filename = "./assets/sketches/ridges.png";
        break;
    case PEAKS_SCREEN:
        filename = "./assets/sketches/peaks.png";
        break;
    case RIVERS_SCREEN:
        filename = "./assets/sketches/rivers.png";
        break;
    case BASINS_SCREEN:
        filename = "./assets/sketches/basins.png";
        break;
    }
    // Save the image as a file
    cv::imwrite(filename, snapshot);

    // Restore the previous viewport
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::cycleDayNight()
{
    // Cycle the day/night cycle using the time variable which sets the rotation for the orbit and the alpha value for the night texture
    this->time += 0.005f * TIME_SPEED;
    if (this->time > 24.f)
        this->time -= 24.0f;
}

void Renderer::setTime(int time)
{
    this->time = time % 24;
}

void Renderer::sketch(float x, float y)
{
    // current page is used as index for the sketch vertices, colors and indices arrays and "-1" removes the case of the landing screen
    short current_canvas = current_menu_page - 1;

    objects[SKETCH + current_canvas].vertices.emplace_back(x);
    objects[SKETCH + current_canvas].vertices.emplace_back(y);
    // The sketch must be drawn together with the canvas; to ensure
    // that the depth buffer is updated correctly, the sketch is drawn with a non 0 z-coordinate
    objects[SKETCH + current_canvas].vertices.emplace_back(0.5);

    // If the current canvas is 0, set color to dark green, if 1 set color to brown, if 2 set color to light blue, if 3 set color dark blue
    if (current_canvas == RIDGES)
    {
        objects[SKETCH + current_canvas].colors.emplace_back(0);
        objects[SKETCH + current_canvas].colors.emplace_back(0.5);
        objects[SKETCH + current_canvas].colors.emplace_back(0);
    }
    else if (current_canvas == PEAKS)
    {
        objects[SKETCH + current_canvas].colors.emplace_back(0.5);
        objects[SKETCH + current_canvas].colors.emplace_back(0.25);
        objects[SKETCH + current_canvas].colors.emplace_back(0);
    }
    else if (current_canvas == RIVERS)
    {
        objects[SKETCH + current_canvas].colors.emplace_back(0.3);
        objects[SKETCH + current_canvas].colors.emplace_back(0.5);
        objects[SKETCH + current_canvas].colors.emplace_back(0.8);
    }
    else if (current_canvas == BASINS)
    {
        objects[SKETCH + current_canvas].colors.emplace_back(0);
        objects[SKETCH + current_canvas].colors.emplace_back(0);
        objects[SKETCH + current_canvas].colors.emplace_back(0.7);
    }
    
    if (x != 0xFFFFFFFFu)
        objects[SKETCH + current_canvas].indices.emplace_back(objects[SKETCH + current_canvas].vertices.size() / 3 - 1);
    else
        objects[SKETCH + current_canvas].indices.emplace_back(0xFFFFFFFFu);
}

void Renderer::resetSketches()
{
    // Clear sketch buffers
    for (short current_canvas = 0; current_canvas < 4; current_canvas++)
    {
        objects[SKETCH + current_canvas].vertices.clear();
        objects[SKETCH + current_canvas].colors.clear();
        objects[SKETCH + current_canvas].indices.clear();
    }
}

void Renderer::animateMenu()
{
    switch (instance->current_menu_page)
    {
    case LANDING_SCREEN:
        if (!instance->menu_clips[LANDING_SCREEN].read(instance->menu_frame))
        {
            instance->menu_clips[LANDING_SCREEN].set(cv::CAP_PROP_POS_FRAMES, 0);
            instance->menu_clips[LANDING_SCREEN].read(instance->menu_frame);
        }
        break;
    case RIDGES_SCREEN:
        if (!instance->menu_clips[RIDGES_SCREEN].read(instance->menu_frame))
        {
            instance->menu_clips[RIDGES_SCREEN].set(cv::CAP_PROP_POS_FRAMES, 0);
            instance->menu_clips[RIDGES_SCREEN].read(instance->menu_frame);
        }
        break;
    case PEAKS_SCREEN:
        if (!instance->menu_clips[PEAKS_SCREEN].read(instance->menu_frame))
        {
            instance->menu_clips[PEAKS_SCREEN].set(cv::CAP_PROP_POS_FRAMES, 0);
            instance->menu_clips[PEAKS_SCREEN].read(instance->menu_frame);
        }
        break;
    case RIVERS_SCREEN:
        if (!instance->menu_clips[RIVERS_SCREEN].read(instance->menu_frame))
        {
            instance->menu_clips[RIVERS_SCREEN].set(cv::CAP_PROP_POS_FRAMES, 0);
            instance->menu_clips[RIVERS_SCREEN].read(instance->menu_frame);
        }
        break;
    case BASINS_SCREEN:
        if (!instance->menu_clips[BASINS_SCREEN].read(instance->menu_frame))
        {
            instance->menu_clips[BASINS_SCREEN].set(cv::CAP_PROP_POS_FRAMES, 0);
            instance->menu_clips[BASINS_SCREEN].read(instance->menu_frame);
        }
        break;
    case LOADING_SCREEN:
        if (!instance->menu_clips[LOADING_SCREEN].read(instance->menu_frame))
        {
            instance->menu_clips[LOADING_SCREEN].set(cv::CAP_PROP_POS_FRAMES, 0);
            instance->menu_clips[LOADING_SCREEN].read(instance->menu_frame);
        }
        break;
    default:
        instance->menu_clips[LOADING_SCREEN].set(cv::CAP_PROP_POS_FRAMES, 0);
        instance->menu_clips[LANDING_SCREEN].set(cv::CAP_PROP_POS_FRAMES, 0);
        instance->menu_frame.release();
        break;
    }
}

void Renderer::timerCallback(int value)
{
    instance->animateMenu();
    instance->cycleDayNight();
    glutPostRedisplay();
    glutTimerFunc(25, Renderer::timerCallback, 0);
}

void Renderer::drawTerrain()
{    
    // Change the lighting color based on the time of day
    if (instance->time > 6 && instance->time < 18)
    {
        GLfloat diffuse_material[] = {1.f, 0.9f, 0.8f, 1.0f}; // Warm color for diffuse light
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_material);
    }
    else
    {
        GLfloat diffuse_material[] = {0.8f, 0.8f, 0.9f, 1.0f}; // Cold color for diffuse light
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_material);
    }
    
    float ambient_light = (1 - std::abs(static_cast<float>(instance->time) / 24.f - 0.5f))*0.25f;
    GLfloat ambient_material[] = {ambient_light, ambient_light, ambient_light, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_material);
    
    // Retrieve the camera position and direction for frustrum culling
    Vec2<float> position = instance->camera->getPosition2D();
    Vec2<float> direction = instance->camera->getDirection2D();
    
    // Draw the terrain using the quadtree frustrum culling
    instance->quadtree->render(position, direction);
}

void Renderer::drawWater()
{
    static float time = 0;
    glEnable(GL_BLEND);
    
    // Bind the water texture
    glBindTexture(GL_TEXTURE_2D, instance->objects[WATER].texture[0]);
    
    // Bind the water VAO
    glBindVertexArray(instance->objects[WATER].vao);
    
    // Enable two vertex arrays: co-ordinates and color.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    
    // Bind the texture buffer object
    glBindBuffer(GL_ARRAY_BUFFER, instance->objects[WATER].tbo);
    // Update textures coordinates
    for (unsigned int i = 0; i < instance->objects[WATER].textures.size(); i += 2)
    {
        instance->objects[WATER].textures[i] += 0.01f;
        instance->objects[WATER].textures[i + 1] += 0.01f;
    }
    glBufferData(GL_ARRAY_BUFFER, instance->objects[WATER].textures.size() * sizeof(float), instance->objects[WATER].textures.data(), GL_STREAM_DRAW);
    
    // Bind the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, instance->objects[WATER].vbo);
    // Perlin noise parameters for macro waves
    vector<float> vertices = instance->objects[WATER].vertices;
    float macro_amplitude = WAVE_MACRO_AMPLITUDE; // Adjust the amplitude to control the wave height
    float macro_frequency = 0.0005f; // Adjust the frequency to control the wave speed
    // Micro wave parameters for sin and cos
    float micro_amplitude = WAVE_MICRO_AMPLITUDE; // Adjust the amplitude of micro waves
    float micro_frequency_x = 0.01f; // Adjust the frequency of micro waves in X direction
    float micro_frequency_z = 0.01f; // Adjust the frequency of micro waves in Z direction
    
    for (unsigned int i = 0; i < instance->objects[WATER].vertices.size(); i += 3)
    {
        float x = instance->objects[WATER].vertices[i];
        float z = instance->objects[WATER].vertices[i + 2];
        float macro_wave = macro_amplitude * instance->perlin_noise.noise3D_01(x * macro_frequency, z * macro_frequency, time);
        float micro_wave_x = micro_amplitude * sin(x * micro_frequency_x + time);
        float micro_wave_z = micro_amplitude * cos(z * micro_frequency_z + time);

        // Combine perlin noise and micro waves
        vertices[i + 1] += macro_wave + micro_wave_x + micro_wave_z;
    }
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);
    
    time += 0.02f;
    
    // Bind the normal buffer object
    glBindBuffer(GL_ARRAY_BUFFER, instance->objects[WATER].nbo);
    // Calculate updated normals for every triangle 
    std::vector<float> normals = instance->objects[WATER].normals;
    for (int i = 0; i < instance->objects[WATER].indices.size() - 3; i += 2)
    {
        if (instance->objects[WATER].indices[i + 1] == 0xFFFFFFFFu)
            continue;

        // Get the indices of the triangle
        int i1 = instance->objects[WATER].indices[i];
        int i2 = instance->objects[WATER].indices[i + 1];
        int i3 = instance->objects[WATER].indices[i + 2];

        // Get the vertices of the triangle into Vec3 objects
        Vec3<float> v1;
        v1.x = vertices[i1 * 3];
        v1.y = vertices[i1 * 3 + 1];
        v1.z = vertices[i1 * 3 + 2];

        Vec3<float> v2;
        v2.x = vertices[i2 * 3];
        v2.y = vertices[i2 * 3 + 1];
        v2.z = vertices[i2 * 3 + 2];

        Vec3<float> v3;
        v3.x = vertices[i3 * 3];
        v3.y = vertices[i3 * 3 + 1];
        v3.z = vertices[i3 * 3 + 2];

        // Calculate normals
        Vec3<float> normal = newellMethod(v1, v2, v3);

        // Add the normal to the normals array
        normals[i1 * 3] += normal.x;
        normals[i1 * 3 + 1] += normal.y;
        normals[i1 * 3 + 2] += normal.z;

        normals[i2 * 3] += normal.x;
        normals[i2 * 3 + 1] += normal.y;
        normals[i2 * 3 + 2] += normal.z;

        normals[i3 * 3] += normal.x;
        normals[i3 * 3 + 1] += normal.y;
        normals[i3 * 3 + 2] += normal.z;
    }
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STREAM_DRAW);
    
    // Disable writing of the frame and depth buffers as only the 
    // stencil buffer need be written next.
    glEnable(GL_STENCIL_TEST); // Enable stencil testing
    glClearStencil(0); // Set clearing value for stencil buffer.
    
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    
    glStencilFunc(GL_ALWAYS, 1, 1); // The stencil test always passes (the reference value and mask are both 1).
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); // In all cases replace the stencil tag
    
    glEnable(GL_PRIMITIVE_RESTART);                                                       
    glDrawElements(GL_TRIANGLE_STRIP, instance->objects[WATER].indices.size(), GL_UNSIGNED_INT, 0);
    glDisable(GL_PRIMITIVE_RESTART);
    
    // Enable writing of the frame and depth buffers - actually drawing now begins.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    
    glStencilFunc(GL_EQUAL, 1, 1); // The stencil test passes only if the corresponding stencil buffer tag is 1.
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // The stencil buffer itself is not updated.	
    
    glPushMatrix();
        glDisable(GL_LIGHTING);
        glScalef(1.0, -1.0, 1.0);
        // Bind the vertex array object for the skydome
        glBindVertexArray(instance->objects[SKYDOME].vao);
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        // Draw the skydome with blending enabled
        glBindTexture(GL_TEXTURE_2D, instance->objects[SKYDOME].texture[0]);
        glDrawElements(GL_TRIANGLES, instance->objects[SKYDOME].indices.size(), GL_UNSIGNED_INT, 0);
        
        // Calculate the alpha value for the night texture
        float normalized_time = static_cast<float>(instance->time) / 24.0f;
        float alpha = 0;
        
        // If sunrise
        if (normalized_time >= 0.25f && normalized_time < 0.33f)
        {
            alpha = static_cast<float>((1.0f - ((normalized_time - 0.25f) / 0.08f)));
        }
        // If day
        else if (normalized_time >= 0.33f && normalized_time < 0.75f)
        {
            alpha = 0;
        }
        // If sunset
        else if (normalized_time >= 0.75f && normalized_time < 0.83f)
        {
            alpha = static_cast<float>(((normalized_time - 0.75f) / 0.08f));
        }
        // If night
        else
        {
            alpha = 1;
        }
        glColor4f(1.0, 1.0, 1.0, alpha); 
        // Draw the skydome with blending enabled
        glBindTexture(GL_TEXTURE_2D, instance->objects[SKYDOME].texture[1]);
        glDrawElements(GL_TRIANGLES, instance->objects[SKYDOME].indices.size(), GL_UNSIGNED_INT, 0);
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        // Unbind the vertex array object and texture
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        instance -> drawOrbit();
        glEnable(GL_LIGHTING);
    glPopMatrix();
    
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    
    glPushMatrix();
        // Change front face to clockwise
        glFrontFace(GL_CW);
        glEnable(GL_CLIP_PLANE0);
        double surface_level = instance->terrain->getWaterLevel() + macro_amplitude + micro_amplitude*2;
        double equation[4] = { 0.0, -1.0, 0.0, surface_level};
        glClipPlane(GL_CLIP_PLANE0, equation);
        glTranslatef(0.0, 2*surface_level, 0.0);
        glScalef(1.0, -1.0, 1.0);
        instance -> drawTerrain();
        // Change front face to counter-clockwise
        glDisable(GL_CLIP_PLANE0);
        glFrontFace(GL_CCW);
    glPopMatrix();
    
    
    glDisable(GL_STENCIL_TEST); // Disable the stencil test
    
    // Bind the water texture
    glBindTexture(GL_TEXTURE_2D, instance->objects[WATER].texture[0]);

    // Bind the water VAO
    glBindVertexArray(instance->objects[WATER].vao);
    
    glEnable(GL_PRIMITIVE_RESTART);                                                                 // Enable primitive restart
    glDrawElements(GL_TRIANGLE_STRIP, instance->objects[WATER].indices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    
    // Unbind the vertex array object and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);

}

void Renderer::drawVegetation()
{
    static float time = 0;
    // Enable blending
    glEnable(GL_BLEND);
    
    // Bind the vegetation texture
    glBindTexture(GL_TEXTURE_2D, instance->objects[VEGETATION].texture[0]);
    
    // Bind the vegetation VAO
    glBindVertexArray(instance->objects[VEGETATION].vao);
    
    // Enable two vertex arrays: coordinates and texture coordinates.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    vector<float> vertices = instance->objects[VEGETATION].vertices;
    
    // Update the vertices such that the vegetation is always facing the camera
    Vec2<float> direction = instance->camera->getDirection2D();
    float radians = atan2(direction.v, direction.u);
    float degrees = abs(fmod(radians * (180.0f / M_PI) + 270.0f, 360.0f) - 180.0f) / 180.0f;
    
    for (unsigned int i = 0; i < instance->objects[VEGETATION].vertices.size(); i += 12)
    {
        vertices[i] -= degrees * BUSH_SIZE * 2;                 // x1
        vertices[i + 2] += direction.u * BUSH_SIZE;             // z1
        vertices[i + 3] -= degrees * BUSH_SIZE * 2;             // x2
        vertices[i + 5] += direction.u * BUSH_SIZE;             // z2
        
        vertices[i + 6] += degrees * BUSH_SIZE * 2;             // x3
        vertices[i + 8] -= direction.u * BUSH_SIZE;             // z3
        vertices[i + 9] += degrees * BUSH_SIZE * 2;             // x4
        vertices[i + 11] -= direction.u * BUSH_SIZE;            // z4
    }
    
    // Bind the VBO and modify the vertices such that the vegetation is always facing the camera
    glBindBuffer(GL_ARRAY_BUFFER, instance->objects[VEGETATION].vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);
    
    vector<float> textures = instance->objects[VEGETATION].textures;
    // Cycle over each texture quad
    for (unsigned int i = 0; i < instance->objects[VEGETATION].textures.size(); i += 8)
    {
        textures[i + 2] += 0.05f * sin(time);
        textures[i + 4] += 0.05f * sin(time);
    }
    
    time+=0.04f;
    
    //Bind the TBO and modify the texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, instance->objects[VEGETATION].tbo);
    glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(float), textures.data(), GL_STREAM_DRAW);
    
    // Draw quads with glDrawArrays
    glDrawArrays(GL_QUADS, 0, vertices.size() / 3);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Unbind the vertex array object and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glDisable(GL_BLEND);
}

void Renderer::drawOrbit()
{  
    float angle = instance->time/24.f*360.f;
    
    glDisable(GL_CULL_FACE);
    // Draw the sun
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        glRotatef(angle, 0, 0, 1);
        
        // Bind the sun texture
        glBindTexture(GL_TEXTURE_2D, instance->objects[SUN].texture[0]);
        
        // Draw the sun
        glBindVertexArray(instance->objects[SUN].vao);
        
        // Enable two vertex arrays: co-ordinates and color.
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glDrawElements(GL_TRIANGLE_STRIP, instance->objects[SUN].indices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        // Unbind the vertex array object and texture
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
    
    // Draw the moon
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        
        glRotatef(angle, 0, 0, 1);

        // Bind the sun texture
        glBindTexture(GL_TEXTURE_2D, instance->objects[MOON].texture[0]);
        
        // Draw the sun
        glBindVertexArray(instance->objects[MOON].vao);
        
        // Enable two vertex arrays: co-ordinates and color.
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glDrawElements(GL_TRIANGLE_STRIP, instance->objects[MOON].indices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        // Unbind the vertex array object and texture
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
    glEnable(GL_CULL_FACE);
}

void Renderer::drawSkydome()
{
    // Enable blending
    glEnable(GL_BLEND);
    
    // Disable depth testing
    glDisable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    
    // Bind the vertex array object for the skydome
    glBindVertexArray(instance->objects[SKYDOME].vao);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Draw the skydome with blending enabled
    glBindTexture(GL_TEXTURE_2D, instance->objects[SKYDOME].texture[0]);
    glDrawElements(GL_TRIANGLES, instance->objects[SKYDOME].indices.size(), GL_UNSIGNED_INT, 0);
    
    // Calculate the alpha value for the night texture
    float normalized_time = static_cast<float>(instance->time) / 24.0f;
    float alpha = 0;
    
    // If sunrise
    if (normalized_time >= 0.25f && normalized_time < 0.33f)
    {
        alpha = static_cast<float>((1.0f - ((normalized_time - 0.25f) / 0.08f)));
    }
    // If day
    else if (normalized_time >= 0.33f && normalized_time < 0.75f)
    {
        alpha = 0;
    }
    // If sunset
    else if (normalized_time >= 0.75f && normalized_time < 0.83f)
    {
        alpha = static_cast<float>(((normalized_time - 0.75f) / 0.08f));
    }
    // If night
    else
    {
        alpha = 1;
    }
    glColor4f(1.0, 1.0, 1.0, alpha);
    
    glBindTexture(GL_TEXTURE_2D, instance->objects[SKYDOME].texture[1]);
    glDrawElements(GL_TRIANGLES, instance->objects[SKYDOME].indices.size(), GL_UNSIGNED_INT, 0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glColor4f(1.0, 1.0, 1.0, 1.0);
    // Unbind the vertex array object and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Re-enable depth testing
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::drawSplashscreen()
{
    if (!instance->menu_frame.empty())
    {
        // Save the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        
        // Set the projection matrix to orthographic
        float width = glutGet(GLUT_WINDOW_WIDTH);
        float height = glutGet(GLUT_WINDOW_HEIGHT);
        glLoadIdentity();
        glOrtho(0, width, 0, height, -1, 1);

        // Update texture data
        glBindTexture(GL_TEXTURE_2D, instance->objects[SPLASHSCREEN].texture[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, instance->menu_frame.cols, instance->menu_frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, instance->menu_frame.data);

        // Update width and height values in a single line
        std::vector<GLfloat> vertices = {0, 0, width, 0, width, height, 0, height};
        // Bind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SPLASHSCREEN].vbo);
        // Update the vertex buffer data
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(GLfloat), vertices.data());

        // Enable the vertex arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Render the splash screen
        glBindVertexArray(instance->objects[SPLASHSCREEN].vao);
        glDrawArrays(GL_QUADS, 0, 4);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        // Restore the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
}

void Renderer::drawCanvas()
{
    if (!instance->menu_frame.empty())
    {
        // Reset the modelview matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Save the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

        // Set the projection matrix to orthographic
        float width = glutGet(GLUT_WINDOW_WIDTH);
        float height = glutGet(GLUT_WINDOW_HEIGHT);
        glLoadIdentity();
        glOrtho(0, width, 0, height, -1, 1);

        // Update texture dataz
        glBindTexture(GL_TEXTURE_2D, instance->objects[CANVAS].texture[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, instance->menu_frame.cols, instance->menu_frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, instance->menu_frame.data);

        // Update width and height values in a single line
        std::vector<GLfloat> vertices = {0, 0, width, 0, width, height, 0, height};

        // Enable the vertex arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Render the splash screen
        glBindVertexArray(instance->objects[CANVAS].vao);

        // Bind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[CANVAS].vbo);
        // Update the vertex buffer data
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
        
        glDrawArrays(GL_QUADS, 0, 4);
        
        glBindVertexArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        // Restore the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
}

void Renderer::drawSketch(short current_canvas)
{
    if (!instance->menu_frame.empty())
    {
        // Reset the modelview matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Save the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
            // Set the projection matrix to orthographic
            float width = glutGet(GLUT_WINDOW_WIDTH);
            float height = glutGet(GLUT_WINDOW_HEIGHT);
            glLoadIdentity();
            glOrtho(0, width, 0, height, -1, 1);

            // Update sketch_vertices to fit the screen: vertices is the non-normalized version of sketch_vertices
            vector<float> vertices(instance->objects[SKETCH + current_canvas].vertices.size());
            
            for (int i = 0; i < instance->objects[SKETCH + current_canvas].vertices.size(); i = i + 3)
            {
                vertices[i] = instance->objects[SKETCH + current_canvas].vertices[i] * width;
                vertices[i + 1] = instance->objects[SKETCH + current_canvas].vertices[i + 1] * height;
                vertices[i + 2] = instance->objects[SKETCH + current_canvas].vertices[i + 2];
            }

            // Enable the vertex arrays
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            // Render the sketch
            glBindVertexArray(instance->objects[SKETCH].vao);
            
            // Bind the vertex buffer object
            glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SKETCH].vbo);
            // Update the vertex buffer data for points
            glBufferData(GL_ARRAY_BUFFER, instance->objects[SKETCH + current_canvas].vertices.size() * sizeof(GLfloat), vertices.data(), GL_STREAM_DRAW);
            // Set the vertexattribute pointer for positions
            glVertexPointer(3, GL_FLOAT, 0, vertices.data());
            
            // Bind the vertex buffer object
            glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SKETCH].cbo);
            // Update the vertex buffer data for points
            glBufferData(GL_ARRAY_BUFFER, instance->objects[SKETCH + current_canvas].colors.size() * sizeof(GLfloat), instance->objects[SKETCH + current_canvas].colors.data(), GL_STREAM_DRAW);
            // Set the vertexattribute pointer for colors
            glColorPointer(3, GL_FLOAT, 0, instance->objects[SKETCH + current_canvas].colors.data());
            
            // Bind the index buffer object
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->objects[SKETCH].ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, instance->objects[SKETCH + current_canvas].indices.size() * sizeof(GLuint), instance->objects[SKETCH + current_canvas].indices.data(), GL_STREAM_DRAW);

            // Enable primitive restart
            glEnable(GL_PRIMITIVE_RESTART);
            glPrimitiveRestartIndex(0xFFFFFFFFu);
            
            // If the current page is rivers or ridges, draw lines
            if (current_canvas == RIDGES || current_canvas == RIVERS)
            {
                // Increment line width
                glLineWidth(5.0f);
                // Draw the sketch using indices
                glDrawElements(GL_LINE_STRIP, instance->objects[SKETCH + current_canvas].indices.size(), GL_UNSIGNED_INT, instance->objects[SKETCH + current_canvas].indices.data());
            }
            if (current_canvas == PEAKS || current_canvas == BASINS)
            {
                // Increment points size
                glPointSize(5.0f);
                // Draw the sketch using indices
                glDrawElements(GL_POINTS, instance->objects[SKETCH + current_canvas].indices.size(), GL_UNSIGNED_INT, instance->objects[SKETCH + current_canvas].indices.data());
            }
            glDisable(GL_PRIMITIVE_RESTART);
            
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            
            // Restore the previous projection matrix
            glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
}

void Renderer::drawTime()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
        
        //get screen size with glut
        int screen_width = glutGet(GLUT_WINDOW_WIDTH);
        int screen_height = glutGet(GLUT_WINDOW_HEIGHT);

        // Set the text position in screen coordinates
        GLfloat text_width = glutStrokeLength(GLUT_STROKE_MONO_ROMAN, reinterpret_cast<const unsigned char *>("hh:mm:ss"));
        GLfloat text_height = glutStrokeHeight(GLUT_STROKE_MONO_ROMAN);
        GLfloat scaling_factor = screen_width / screen_height * 0.2;
        GLfloat text_pos_x = (screen_width - text_width * scaling_factor) / 2.0f; // Center horizontally
        GLfloat text_pos_y = screen_height - 80.0f;                  // Position at the top with 80 pixels offset
        
        // Set up an orthographic projection
        glOrtho(0, screen_width, 0, screen_height, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
            glLoadIdentity();                                                                                               // Position at the top with 30 pixels offset
            
            // Set the color for the text
            glColor3f(1.0f, 1.0f, 1.0f); // White color
            
            // Convert the time to a string. Divide by 360 to normalize the time [0,1] and then multiply by to 24 hours, 60 minutes and 60 seconds.
            int total_seconds = static_cast<int>(instance->time * 60.0f * 60.0f);
            // Calculate hours, minutes, and seconds
            int hours = total_seconds / 3600;
            int minutes = (total_seconds % 3600) / 60;
            int seconds = total_seconds % 60;
            
            // Format the time as a string
            char time_string[10]; // "hh:mm:ss" + '\0'
            snprintf(time_string, sizeof(time_string), "%02d:%02d:%02d", hours, minutes, seconds);
            
            // Move to the desired position in screen coordinates
            glTranslatef(text_pos_x, text_pos_y, 0.0f);

            // Scale down the text
            glScalef(scaling_factor, scaling_factor, scaling_factor);
            
            // Display the time string
            for (const char *c = time_string; *c != '\0'; c++)
                glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Renderer::renderLight()
{
    // Set the light position based on the mesh size
    static float diffuse_light_y = instance->terrain->getWorldDim()/2;
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        // Draw the light source
        GLfloat light_position[4];
        // Initialize the light position to the sun's position
        light_position[0] = 0;
        light_position[2] = 0;
        light_position[3] = 1;
        
        GLfloat spot_direction[3] = {0.0f, -1.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

        // If daytime, use diffuse sunlight
        if (instance->time > 6 && instance->time < 18)
        {
            float light[] = {1.0, 0.9, 0.8, 1.0};

            glLightfv(GL_LIGHT0, GL_DIFFUSE, light);
            light_position[1] = diffuse_light_y;
        }
        // If nighttime, use diffuse moonlight
        else
        {
            float light[] = {0.25, 0.5, 0.75, 1.0};
            glLightfv(GL_LIGHT0, GL_DIFFUSE, light);
            light_position[1] = -diffuse_light_y;
        }
        
        float angle = instance->time / 24.f * 360 + 180;
        glRotatef(angle, 0, 0, 1);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glPopMatrix();
}

void Renderer::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    // Set the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Update the camera based on the inputs
    instance->camera->update();
    
    // Switch on the current menu page
    switch (instance->current_menu_page)
    {
    case LANDING_SCREEN:
        instance->drawSplashscreen();
        break;
    case RENDERING_SCREEN:
        instance->drawSkydome();
        instance->drawOrbit();
        glEnable(GL_LIGHTING);
        instance->drawWater();
        instance->drawTerrain();
        instance->drawVegetation();
        instance->renderLight();
        glDisable(GL_LIGHTING);
        instance->drawTime();
        break;
    case LOADING_SCREEN:
        instance->drawCanvas();
        break;
    case RIDGES_SCREEN:
        instance->drawCanvas();
        instance->drawSketch(RIDGES);
        break;
    case PEAKS_SCREEN:
        instance->drawCanvas();
        instance->drawSketch(PEAKS);
        instance->drawSketch(RIDGES);
        break;
    case RIVERS_SCREEN:
        instance->drawCanvas();
        instance->drawSketch(RIVERS);
        instance->drawSketch(PEAKS);
        instance->drawSketch(RIDGES);
        break;
    case BASINS_SCREEN:
        instance->drawCanvas();
        instance->drawSketch(BASINS);
        instance->drawSketch(RIVERS);
        instance->drawSketch(PEAKS);
        instance->drawSketch(RIDGES);
        break;
    }
    
    glutSwapBuffers();
}