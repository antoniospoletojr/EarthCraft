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
    // Disable the vertexarrays
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    
    // Unbind any buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // Deallocate memory
    objects.clear();
    
    // Delete the vertexarray objects
    glDeleteVertexArrays(1, &objects[MESH].vao);
    glDeleteVertexArrays(1, &objects[SUN].vao);
    glDeleteVertexArrays(1, &objects[MOON].vao);
    glDeleteVertexArrays(1, &objects[SPLASHSCREEN].vao);
    glDeleteVertexArrays(1, &objects[CANVAS].vao);

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

void Renderer::initializeMesh(Terrain *terrain, int mesh_multiplier)
{    
    this->terrain = terrain;
    // Print the map info
    this->terrain->getInfo();
    // Retrieve the map
    Vec3<float> *map = this->terrain->getMap();
    // Load skydome texture image
    cv::Mat mesh_texture = this->terrain->getTexture();
    // Get the dimension of the map, useful for allocations
    int dim = this->terrain->getDim();
    // Get the world dimension of the map, useful for texture mapping
    float world_dim = this->terrain->getWorldDim();

    // Generate and bind a texture object
    glGenTextures(1, &objects[MESH].texture);
    glBindTexture(GL_TEXTURE_2D, objects[MESH].texture);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mesh_texture.cols, mesh_texture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, mesh_texture.data);

    // Generate the vertexarray object for the mesh
    glGenVertexArrays(1, &objects[MESH].vao);
    // Bind the vertexarray object for the mesh
    glBindVertexArray(objects[MESH].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[MESH].vbo);
    glGenBuffers(1, &objects[MESH].tbo);
    glGenBuffers(1, &objects[MESH].ibo);
    glGenBuffers(1, &objects[MESH].nbo);
    
    // Reset mesh arrays if they are not empty
    objects[MESH].vertices.clear();
    objects[MESH].indices.clear();
    objects[MESH].textures.clear();
    objects[MESH].normals.clear();
    
    // Generate vertices and colors
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            objects[MESH].vertices.push_back(map[i * dim + j].x);
            objects[MESH].vertices.push_back(map[i * dim + j].y);
            objects[MESH].vertices.push_back(map[i * dim + j].z);
            
            objects[MESH].textures.push_back((float)i / dim);
            objects[MESH].textures.push_back((float)j / dim);
            
            objects[MESH].normals.push_back(0.0f);
            objects[MESH].normals.push_back(0.0f);
            objects[MESH].normals.push_back(0.0f);
        }
    }
    
    // Generate indices for triangle strips
    for (int z = 0; z < dim - 1; z++)
    {
        // Start a new strip
        objects[MESH].indices.push_back(z * dim);
        for (int x = 0; x < dim; x++)
        {
            // Add vertices to strip
            objects[MESH].indices.push_back(z * dim + x);
            objects[MESH].indices.push_back((z + 1) * dim + x);
        }
        // Use primitive restart to start a new strip
        objects[MESH].indices.push_back(0xFFFFFFFFu);
    }
    
    // Calculate normals
    for (int i = 0; i < objects[MESH].indices.size()-3; i += 2)
    {    
        if (objects[MESH].indices[i+1] == 0xFFFFFFFFu)
            continue;
        
        // Get the indices of the triangle
        int i1 = objects[MESH].indices[i];
        int i2 = objects[MESH].indices[i + 1];
        int i3 = objects[MESH].indices[i + 2];
        
        // Get the vertices of the triangle into Vec3 objects
        Vec3<float> v1;
        v1.x = objects[MESH].vertices[i1 * 3];
        v1.y = objects[MESH].vertices[i1 * 3 + 1];
        v1.z = objects[MESH].vertices[i1 * 3 + 2];
        
        Vec3<float> v2;
        v2.x = objects[MESH].vertices[i2 * 3];
        v2.y = objects[MESH].vertices[i2 * 3 + 1];
        v2.z = objects[MESH].vertices[i2 * 3 + 2];
        
        Vec3<float> v3;
        v3.x = objects[MESH].vertices[i3 * 3];
        v3.y = objects[MESH].vertices[i3 * 3 + 1];
        v3.z = objects[MESH].vertices[i3 * 3 + 2];
        
        // Get the vertices of the triangle
        Vec3<float> u1 = subtract(v2, v1);
        Vec3<float> u2 = subtract(v3, v1);
        
        // Calculate the normal of the triangle
        Vec3<float> normal = crossProduct(u1, u2);
        
        // // Add the normal to the normals array
        objects[MESH].normals[i1 * 3] += normal.x;
        objects[MESH].normals[i1 * 3 + 1] += normal.y;
        objects[MESH].normals[i1 * 3 + 2] += normal.z;
        
        objects[MESH].normals[i2 * 3] += normal.x;
        objects[MESH].normals[i2 * 3 + 1] += normal.y;
        objects[MESH].normals[i2 * 3 + 2] += normal.z;
        
        objects[MESH].normals[i3 * 3] += normal.x;
        objects[MESH].normals[i3 * 3 + 1] += normal.y;
        objects[MESH].normals[i3 * 3 + 2] += normal.z;
    }
    
    // If mesh_multiplier is greater than 0, the mesh will be replicated
    if (mesh_multiplier>0)
    {
        // Create a temporary vector for updated vertex positions
        std::vector<GLfloat> updated_vertices, updated_normals, updated_textures;
        std::vector<GLuint> updated_indices;

        for (int i = -mesh_multiplier; i <= mesh_multiplier; i++)
        {
            for (int j = -mesh_multiplier; j <= mesh_multiplier; j++)
            {
                for (size_t k = 0; k < instance->objects[MESH].vertices.size(); k += 3)
                {
                    updated_vertices.push_back(instance->objects[MESH].vertices[k] + world_dim * i);        // Update x coordinate
                    updated_vertices.push_back(instance->objects[MESH].vertices[k + 1]);                    // Keep y coordinate unchanged
                    updated_vertices.push_back(instance->objects[MESH].vertices[k + 2] + world_dim * j);    // Update z coordinate

                    updated_normals.push_back(instance->objects[MESH].normals[k]);                          // Keep x coordinate unchanged
                    updated_normals.push_back(instance->objects[MESH].normals[k + 1]);                      // Keep y coordinate unchanged
                    updated_normals.push_back(instance->objects[MESH].normals[k + 2]);                      // Keep z coordinate unchanged
                }

                for (size_t k = 0; k < instance->objects[MESH].textures.size(); k += 2)
                {
                    updated_textures.push_back(instance->objects[MESH].textures[k]);                        // Keep x coordinate unchanged
                    updated_textures.push_back(instance->objects[MESH].textures[k + 1]);                    // Keep y coordinate unchanged
                }

                short step_number = (i + mesh_multiplier) * (mesh_multiplier * 2 + 1) + (j + mesh_multiplier);
                for (size_t k = 0; k < instance->objects[MESH].indices.size(); k++)
                {
                    updated_indices.push_back(instance->objects[MESH].indices[k] + instance->objects[MESH].vertices.size()/3 * step_number);
                }
            }
        }
        objects[MESH].vertices = updated_vertices;
        objects[MESH].normals = updated_normals;
        objects[MESH].textures = updated_textures;
        objects[MESH].indices = updated_indices;
    }
            
    // Use maximum unsigned int as restart index
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFFu);
    
    // Bind and fill the vertexbuffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MESH].vbo);
    glBufferData(GL_ARRAY_BUFFER, objects[MESH].vertices.size() * sizeof(float), objects[MESH].vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    
    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MESH].tbo);
    glBufferData(GL_ARRAY_BUFFER, objects[MESH].textures.size() * sizeof(float), objects[MESH].textures.data(), GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
    
    // Bind and fill the normals buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MESH].nbo);
    glBufferData(GL_ARRAY_BUFFER, objects[MESH].normals.size() * sizeof(float), objects[MESH].normals.data(), GL_STATIC_DRAW);
    glNormalPointer(GL_FLOAT, 0, 0);
    
    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[MESH].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, objects[MESH].indices.size() * sizeof(GLuint), objects[MESH].indices.data(), GL_STATIC_DRAW);
    
    // Unbind everything
    glBindVertexArray(0);
}

void Renderer::initializeOrbit()
{
    // SUN OBJECT
    // Load skydome texture image
    cv::Mat sun_texture = cv::imread("./assets/textures/sun.png");

    // Check if the image was loaded successfully
    if (sun_texture.empty())
    {
        // Handle error
        std::cerr << "Failed to load texture image." << std::endl;
        return;
    }

    // Generate and bind a texture object
    glGenTextures(1, &objects[SUN].texture);
    glBindTexture(GL_TEXTURE_2D, objects[SUN].texture);

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
            objects[SUN].vertices.push_back(Vec.y - 25000);
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
    
    // Generate the vertexarray object for the sun
    glGenVertexArrays(1, &objects[SUN].vao);
    // Bind the vertexarray object for the sun
    glBindVertexArray(objects[SUN].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[SUN].vbo);
    glGenBuffers(1, &objects[SUN].tbo);
    glGenBuffers(1, &objects[SUN].ibo);

    // Bind and fill the vertexbuffer object
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
    glGenTextures(1, &objects[MOON].texture);
    glBindTexture(GL_TEXTURE_2D, objects[MOON].texture);

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
            objects[MOON].vertices.push_back(Vec.y + 25000);
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

    // Generate the vertexarray object for the sun
    glGenVertexArrays(1, &objects[MOON].vao);
    // Bind the vertexarray object for the sun
    glBindVertexArray(objects[MOON].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[MOON].vbo);
    glGenBuffers(1, &objects[MOON].tbo);
    glGenBuffers(1, &objects[MOON].ibo);

    // Bind and fill the vertexbuffer object
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
    instance->day_texture = cv::imread("./assets/textures/day.jpg");
    
    // Check if the image was loaded successfully
    if (instance->day_texture.empty())
    {
        // Handle error
        std::cerr << "Failed to load skydome texture image." << std::endl;
        return;
    }

    // Add alpha channel to day_texture in place
    cv::cvtColor(instance->day_texture, instance->day_texture, cv::COLOR_BGR2BGRA);

    // Load night texture image
    instance->night_texture = cv::imread("./assets/textures/night.jpg");

    // Check if the image was loaded successfully
    if (instance->night_texture.empty())
    {
        // Handle error
        std::cerr << "Failed to load night texture image." << std::endl;
        return;
    }

    // Add alpha channel to night_texture in place
    cv::cvtColor(instance->night_texture, instance->night_texture, cv::COLOR_BGR2BGRA);

    // Generate and bind a texture object
    glGenTextures(1, &objects[SKYDOME].texture);
    glBindTexture(GL_TEXTURE_2D, objects[SKYDOME].texture);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->day_texture.cols, instance->day_texture.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, instance->day_texture.data);

    // Generate and bind a texture object for the night texture
    glGenTextures(1, &objects[SKYDOME].blend_texture);
    glBindTexture(GL_TEXTURE_2D, objects[SKYDOME].blend_texture);
    
    // Set texture parameters for the night texture (similar to day texture parameters)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload the night texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->night_texture.cols, instance->night_texture.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, instance->night_texture.data);

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
            objects[SKYDOME].vertices.push_back(Vec.x);
            objects[SKYDOME].vertices.push_back(Vec.y);
            objects[SKYDOME].vertices.push_back(Vec.z);

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

    // Generate the vertexarray object for the skydome
    glGenVertexArrays(1, &objects[SKYDOME].vao);
    
    // Bind the vertexarray object for the skydome
    glBindVertexArray(objects[SKYDOME].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[SKYDOME].vbo);
    glGenBuffers(1, &objects[SKYDOME].tbo);
    glGenBuffers(1, &objects[SKYDOME].ibo);

    // Bind and fill the vertexbuffer object
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

    // Generate the vertexarray object for the SPLASHSCREEN
    glGenVertexArrays(1, &objects[SPLASHSCREEN].vao);
    // Bind the vertexarray object for the SPLASHSCREEN
    glBindVertexArray(objects[SPLASHSCREEN].vao);
    
    // Generate the vertexbuffer objects
    glGenBuffers(1, &objects[SPLASHSCREEN].vbo);
    // Generate the texture buffer objects
    glGenBuffers(1, &objects[SPLASHSCREEN].tbo);
    // Generate the texture buffer objects
    glGenBuffers(1, &objects[SPLASHSCREEN].cbo);

    // Create vertexdata for the quad
    std::vector<GLfloat> vertices(8);
    std::vector<GLfloat> colors = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    std::vector<GLfloat> texture_coords = {0, 1, 1, 1, 1, 0, 0, 0};

    // Enable the vertexarrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Bind the vertexbuffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SPLASHSCREEN].vbo);
    // Copy data into the vertexbuffer
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

    glGenTextures(1, &objects[SPLASHSCREEN].texture);
}

void Renderer::initializeCanvas()
{
    // Load assets for the canvas pages
    instance->menu_clips[RIDGES_SCREEN].open("./assets/menu/Ridges.mp4");
    instance->menu_clips[PEAKS_SCREEN].open("./assets/menu/Peaks.mp4");
    instance->menu_clips[RIVERS_SCREEN].open("./assets/menu/Rivers.mp4");
    instance->menu_clips[BASINS_SCREEN].open("./assets/menu/Basins.mp4");
    instance->menu_clips[LOADING_SCREEN].open("./assets/menu/Loadingscreen.mp4");

    // Generate the vertexarray object for the canvas
    glGenVertexArrays(1, &objects[CANVAS].vao);
    // Bind the vertexarray object for the canvas
    glBindVertexArray(objects[CANVAS].vao);

    // Generate the vertexbuffer objects
    glGenBuffers(1, &objects[CANVAS].vbo);
    // Generate the texture buffer objects
    glGenBuffers(1, &objects[CANVAS].tbo);
    // Generate the texture buffer objects
    glGenBuffers(1, &objects[CANVAS].cbo);

    // Create vertexdata for the quad
    std::vector<GLfloat> vertices(8);
    std::vector<GLfloat> colors = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    std::vector<GLfloat> texture_coords = {0, 1, 1, 1, 1, 0, 0, 0};

    // Enable the vertexarrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Bind the vertexbuffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[CANVAS].vbo);
    // Copy data into the vertexbuffer
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

    glGenTextures(1, &objects[CANVAS].texture);
}

void Renderer::initialize(Camera *camera)
{
    // Set the camera
    this->camera = camera;

    // Allocate space for 7 objects (Mesh, Splashscreen, Canvas, Skydome, Sun, Moon and the 4 sketches)
    objects.resize(10);

    // Generate the vertexarray objects; we need 2 objects: MESH and ORBIT
    this->initializeOrbit();
    this->initializeSplashscreen();
    this->initializeCanvas();
    this->initializeSkydome();

    // Set the glut timer callback for the sun animaton
    glutTimerFunc(100, Renderer::timerCallback, 0);

    // Set the glut display callback
    glutDisplayFunc(Renderer::draw);
}

void Renderer::takeSnapshot()
{
    short current_canvas = current_menu_page - 1;

    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport); // Save the previous viewport

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // generate texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    glViewport(0, 0, 1920, 1080);
    instance->drawSketch(current_canvas);

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
    // Cycle the day/night cycle using the time variabl which sets the rotation for the orbit and the alpha value for the night texture
    this->time += 0.1f;
    if (this->time > 24.f)
        this->time -= 24.0f;
    
    // Calculate the alpha value for the night texture
    float normalized_time = static_cast<float>(this->time) / 24.0f;
    uchar alpha = 0;
    
    // If sunrise
    if (normalized_time >= 0.25f && normalized_time < 0.33f)
    {
        alpha = static_cast<uchar>((1.0f - ((normalized_time - 0.25f) / 0.08f)) * 255);
    }
    // If day
    else if (normalized_time >= 0.33f && normalized_time < 0.75f)
    {
        alpha = 0;
    }
    // If sunset
    else if (normalized_time >= 0.75f && normalized_time < 0.83f)
    {
        alpha = static_cast<uchar>(((normalized_time - 0.75f) / 0.08f) * 255);
    }
    // If night
    else
    {
        alpha = 255;
    }
    
    // Set the alpha value for the night texture such that it is transparent during the day and opaque during the night
    for (int y = 0; y < instance->night_texture.rows; ++y)
    {
        cv::Vec4b *pixelRow = instance->night_texture.ptr<cv::Vec4b>(y);
        for (int x = 0; x < instance->night_texture.cols; ++x)
        {
            pixelRow[x][3] = alpha;
        }
    }
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
    // current page is used as index for the sketch vertices, colors and indices arrays
    for (short current_canvas = 0; current_canvas < 4; current_canvas++)
    {
        objects[SKETCH + current_canvas].vertices.clear();
        objects[SKETCH + current_canvas].colors.clear();
        objects[SKETCH + current_canvas].indices.clear();
    }
}

void Renderer::timerCallback(int value)
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

    instance->cycleDayNight();
    glutPostRedisplay();
    glutTimerFunc(25, Renderer::timerCallback, 0);
}

void Renderer::drawMesh()
{
    // Bind the terrain texture
    glBindTexture(GL_TEXTURE_2D, instance->objects[MESH].texture);
    
    // Draw the terrain
    glBindVertexArray(instance->objects[MESH].vao);
    
    // Enable two vertexarrays: co-ordinates and color.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    
    // If daytime, use the diffuse
    if (instance->time > 6 && instance->time < 18)
    {
        GLfloat diffuse_material[] = {1.f, 0.9f, 0.8f, 1.0f}; // Warm color for diffuse light
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_material);
    }
    else
    {
        GLfloat diffuse_material[] = {0.8f, 0.8f, 0.8f, 1.0f}; // Cold color for diffuse light
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_material);
    }

    
    float ambient_light = (1 - std::abs(static_cast<float>(instance->time) / 24.f - 0.5f))*0.2f;
    GLfloat ambient_material[] = {ambient_light, ambient_light, ambient_light, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_material);
    
    glEnable(GL_PRIMITIVE_RESTART);                                                                 // Enable primitive restart
    glDrawElements(GL_QUAD_STRIP, instance->objects[MESH].indices.size(), GL_UNSIGNED_INT, 0);  // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    
    // Unbind the vertexarray object and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::drawOrbit()
{
    GLfloat ambient_material[] = {1, 1, 1, 1.0f}; // Warm color for diffuse reflection
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_material);
    
    float angle = instance->time/24.f*360.f;

    // Draw the sun
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        glRotatef(angle, 0, 0, 1);
        
        // Bind the sun texture
        glBindTexture(GL_TEXTURE_2D, instance->objects[SUN].texture);
        
        // Draw the sun
        glBindVertexArray(instance->objects[SUN].vao);

        // Enable two vertexarrays: co-ordinates and color.
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glDrawElements(GL_TRIANGLE_STRIP, instance->objects[SUN].indices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        // Unbind the vertexarray object and texture
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
    
    // Draw the moon
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

        glRotatef(angle, 0, 0, 1);

        // Bind the sun texture
        glBindTexture(GL_TEXTURE_2D, instance->objects[MOON].texture);
        
        // Draw the sun
        glBindVertexArray(instance->objects[MOON].vao);
        
        // Enable two vertexarrays: co-ordinates and color.
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glDrawElements(GL_TRIANGLE_STRIP, instance->objects[MOON].indices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        // Unbind the vertexarray object and texture
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}

void Renderer::drawSkydome()
{
    // Disable depth testing
    glDisable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);

    // Bind the vertexarray object for the skydome
    glBindVertexArray(instance->objects[SKYDOME].vao);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Draw the skydome with blending enabled
    glBindTexture(GL_TEXTURE_2D, instance->objects[SKYDOME].texture);
    glDrawElements(GL_TRIANGLES, instance->objects[SKYDOME].indices.size(), GL_UNSIGNED_INT, 0);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindTexture(GL_TEXTURE_2D, instance->objects[SKYDOME].blend_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->night_texture.cols, instance->night_texture.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, instance->night_texture.data);
    glDrawElements(GL_TRIANGLES, instance->objects[SKYDOME].indices.size(), GL_UNSIGNED_INT, 0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Unbind the vertexarray object and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Re-enable depth testing
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
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
        glBindTexture(GL_TEXTURE_2D, instance->objects[SPLASHSCREEN].texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, instance->menu_frame.cols, instance->menu_frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, instance->menu_frame.data);

        // Update width and height values in a single line
        std::vector<GLfloat> vertices = {0, 0, width, 0, width, height, 0, height};
        // Bind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SPLASHSCREEN].vbo);
        // Update the vertex buffer data
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(GLfloat), vertices.data());

        // Enable the vertexarrays
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
        glBindTexture(GL_TEXTURE_2D, instance->objects[CANVAS].texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, instance->menu_frame.cols, instance->menu_frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, instance->menu_frame.data);

        // Update width and height values in a single line
        std::vector<GLfloat> vertices = {0, 0, width, 0, width, height, 0, height};

        // Enable the vertexarrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Render the splash screen
        glBindVertexArray(instance->objects[CANVAS].vao);

        // Bind the vertexbuffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[CANVAS].vbo);
        // Update the vertexbuffer data
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

        // Enable the vertexarrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Render the sketch
        glBindVertexArray(instance->objects[SKETCH].vao);
        
        // Bind the vertexbuffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SKETCH].vbo);
        // Update the vertexbuffer data for points
        glBufferData(GL_ARRAY_BUFFER, instance->objects[SKETCH + current_canvas].vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
        // Set the vertexattribute pointer for positions
        glVertexPointer(3, GL_FLOAT, 0, vertices.data());
        
        // Bind the vertexbuffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SKETCH].cbo);
        // Update the vertexbuffer data for points
        glBufferData(GL_ARRAY_BUFFER, instance->objects[SKETCH + current_canvas].colors.size() * sizeof(GLfloat), instance->objects[SKETCH + current_canvas].colors.data(), GL_STATIC_DRAW);
        // Set the vertexattribute pointer for colors
        glColorPointer(3, GL_FLOAT, 0, instance->objects[SKETCH + current_canvas].colors.data());

        // Bind the index buffer object
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->objects[SKETCH].ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, instance->objects[SKETCH + current_canvas].indices.size() * sizeof(GLuint), instance->objects[SKETCH + current_canvas].indices.data(), GL_STATIC_DRAW);
        
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
        
        // Deallocate memory
        vertices.clear();
        
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
    static float diffuse_light_y = instance->terrain->getWorldDim()/2 * (REPLICATION_FACTOR*2 + 1);

    GLfloat ambient_light_position[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, ambient_light_position);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        // Draw the light source
        GLfloat diffuse_light_position[4];
        // Initialize the light position to the sun's position
        diffuse_light_position[0] = 0;
        diffuse_light_position[2] = 0;
        diffuse_light_position[3] = 1;

        GLfloat spot_direction[3] = {0.0f, -1.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
        
        GLfloat cutoff_angle = 90.0f;
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, cutoff_angle);

        GLfloat exponent_value = 1.f;
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, exponent_value);
        
        // If daytime, use the diffuse sunlight
        if (instance->time > 6 && instance->time < 18)
        {
            float diffuse_light[] = {1.0, 0.9, 0.8, 1.0};
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
            diffuse_light_position[1] = diffuse_light_y;
        }
        // If nighttime, use the diffuse moonlight
        else
        {
            float diffuse_light[] = {0.25, 0.5, 0.75, 1.0};
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
            diffuse_light_position[1] = -diffuse_light_y;
        }
        
        
        float angle = instance->time / 24.f * 360 + 180;
        glRotatef(angle, 0, 0, 1);
        glLightfv(GL_LIGHT0, GL_POSITION, diffuse_light_position);

        glPopMatrix();
}

void Renderer::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    
    // Set the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Update the camera based on the inputs
    instance->camera->update();
    
    // Switch on the current menu page ---------MAYBE USELESS!!!!!
    switch (instance->current_menu_page)
    {
    case LANDING_SCREEN:
        instance->drawSplashscreen();
        break;
    case RENDERING_SCREEN:
        instance->drawSkydome();
        instance->drawOrbit();
        instance->drawTime();
        glEnable(GL_LIGHTING);
        instance->drawMesh();
        instance->renderLight();
        glDisable(GL_LIGHTING);
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