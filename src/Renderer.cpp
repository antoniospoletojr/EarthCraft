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
    mesh_vertices.clear();
    mesh_indices.clear();
    sun_vertices.clear();
    sun_indices.clear();
    mesh_normals.clear();
    

    for (auto &vertices : sketch_vertices)
        vertices.clear();
    for (auto &vertices : sketch_colors)
        vertices.clear();
    for (auto &vertices : sketch_indices)
        vertices.clear();

    objects.clear();
    
    // Delete the vertex array objects
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

void Renderer::initializeMesh(Terrain *terrain)
{    
    this->terrain = terrain;
    // Print the map info
    this->terrain->getInfo();
    // Retrieve the map
    Vertex3d<float> *map = this->terrain->getMap();

    // Load skydome texture image
    cv::Mat mesh_texture = this->terrain->getTexture();
    printf("Mesh texture size: %d x %d\n", mesh_texture.cols, mesh_texture.rows);
    fflush(stdout);

    // Get the dimension of the map, useful for allocations
    int dim = this->terrain->getDim();
    
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

    // Generate the vertex array object for the mesh
    glGenVertexArrays(1, &objects[MESH].vao);
    // Bind the vertex array object for the mesh
    glBindVertexArray(objects[MESH].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[MESH].vbo);
    glGenBuffers(1, &objects[MESH].tbo);
    glGenBuffers(1, &objects[MESH].ibo);
    glGenBuffers(1, &objects[MESH].nbo);

    // Reset mesh arrays if they are not empty
    mesh_vertices.clear();
    mesh_indices.clear();
    mesh_textures.clear();
    mesh_normals.clear();
    
    // Generate vertices and colors
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            mesh_vertices.push_back(map[i * dim + j].x);
            mesh_vertices.push_back(map[i * dim + j].y);
            mesh_vertices.push_back(map[i * dim + j].z);
            
            mesh_textures.push_back((float)i / dim);
            mesh_textures.push_back((float)j / dim);
            
            mesh_normals.push_back(0.0f);
            mesh_normals.push_back(0.0f);
            mesh_normals.push_back(0.0f);
        }
    }
    
    // Generate indices for triangle strips
    for (int z = 0; z < dim - 1; z++)
    {
        // Start a new strip
        mesh_indices.push_back(z * dim);
        for (int x = 0; x < dim; x++)
        {
            // Add vertices to strip
            mesh_indices.push_back(z * dim + x);
            mesh_indices.push_back((z + 1) * dim + x);
        }
        // Use primitive restart to start a new strip
        mesh_indices.push_back(0xFFFFFFFFu);
    }
    
    
    // Calculate normals
    for (int i = 0; i < mesh_indices.size()-3; i += 2)
    {    
        if (mesh_indices[i+1] == 0xFFFFFFFFu)
            continue;
        
        // Get the indices of the triangle
        int i1 = mesh_indices[i];
        int i2 = mesh_indices[i + 1];
        int i3 = mesh_indices[i + 2];
        
        // Get the vertices of the triangle into Vertex3d objects
        Vertex3d<float> v1;
        v1.x = mesh_vertices[i1 * 3];
        v1.y = mesh_vertices[i1 * 3 + 1];
        v1.z = mesh_vertices[i1 * 3 + 2];
        
        Vertex3d<float> v2;
        v2.x = mesh_vertices[i2 * 3];
        v2.y = mesh_vertices[i2 * 3 + 1];
        v2.z = mesh_vertices[i2 * 3 + 2];
        
        Vertex3d<float> v3;
        v3.x = mesh_vertices[i3 * 3];
        v3.y = mesh_vertices[i3 * 3 + 1];
        v3.z = mesh_vertices[i3 * 3 + 2];
        
        // Get the vertices of the triangle
        Vertex3d<float> u1 = subtract(v2, v1);
        Vertex3d<float> u2 = subtract(v3, v1);
        
        // Calculate the normal of the triangle
        Vertex3d<float> normal = crossProduct(u1, u2);
        
        // // Add the normal to the normals array
        mesh_normals[i1 * 3] += normal.x;
        mesh_normals[i1 * 3 + 1] += normal.y;
        mesh_normals[i1 * 3 + 2] += normal.z;
        
        mesh_normals[i2 * 3] += normal.x;
        mesh_normals[i2 * 3 + 1] += normal.y;
        mesh_normals[i2 * 3 + 2] += normal.z;
        
        mesh_normals[i3 * 3] += normal.x;
        mesh_normals[i3 * 3 + 1] += normal.y;
        mesh_normals[i3 * 3 + 2] += normal.z;
    }
        
    // Use maximum unsigned int as restart index
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFFu);

    // Bind and fill the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MESH].vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_vertices.size() * sizeof(float), mesh_vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MESH].tbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_textures.size() * sizeof(float), mesh_textures.data(), GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
    
    // Bind and fill the normals buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MESH].nbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_normals.size() * sizeof(float), mesh_normals.data(), GL_STATIC_DRAW);
    glNormalPointer(GL_FLOAT, 0, 0);
    
    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[MESH].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_indices.size() * sizeof(GLuint), mesh_indices.data(), GL_STATIC_DRAW);

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

        // For each vertex in the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            // Extract vertices
            aiVector3D vertex = mesh->mVertices[j];
            sun_vertices.push_back(vertex.x);
            sun_vertices.push_back(vertex.y + 5000);
            sun_vertices.push_back(vertex.z);

            // Extract texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                sun_textures.push_back(texCoord.x);
                sun_textures.push_back(texCoord.y);
            }
        }

        // For each face in the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace &face = mesh->mFaces[j];

            // Assume triangular faces
            if (face.mNumIndices == 3)
            {
                sun_indices.push_back(face.mIndices[0]);
                sun_indices.push_back(face.mIndices[1]);
                sun_indices.push_back(face.mIndices[2]);
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
    glBufferData(GL_ARRAY_BUFFER, sun_vertices.size() * sizeof(float), sun_vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SUN].tbo);
    glBufferData(GL_ARRAY_BUFFER, sun_textures.size() * sizeof(float), sun_textures.data(), GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[SUN].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sun_indices.size() * sizeof(GLuint), sun_indices.data(), GL_STATIC_DRAW);

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

        // For each vertex in the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            // Extract vertices
            aiVector3D vertex = mesh->mVertices[j];
            moon_vertices.push_back(vertex.x);
            moon_vertices.push_back(vertex.y - 5000);
            moon_vertices.push_back(vertex.z);

            // Extract texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                moon_textures.push_back(texCoord.x);
                moon_textures.push_back(texCoord.y);
            }
        }

        // For each face in the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace &face = mesh->mFaces[j];

            // Assume triangular faces
            if (face.mNumIndices == 3)
            {
                moon_indices.push_back(face.mIndices[0]);
                moon_indices.push_back(face.mIndices[1]);
                moon_indices.push_back(face.mIndices[2]);
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
    glBufferData(GL_ARRAY_BUFFER, moon_vertices.size() * sizeof(float), moon_vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MOON].tbo);
    glBufferData(GL_ARRAY_BUFFER, moon_textures.size() * sizeof(float), moon_textures.data(), GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[MOON].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, moon_indices.size() * sizeof(GLuint), moon_indices.data(), GL_STATIC_DRAW);

    // Unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::initializeSkydome()
{
    // Load skydome texture image
    cv::Mat skydome_texture = cv::imread("./assets/textures/skydome.jpg");

    // Check if the image was loaded successfully
    if (skydome_texture.empty())
    {
        // Handle error
        std::cerr << "Failed to load skydome texture image." << std::endl;
        return;
    }

    // Generate and bind a texture object
    glGenTextures(1, &objects[SKYDOME].texture);
    glBindTexture(GL_TEXTURE_2D, objects[SKYDOME].texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skydome_texture.cols, skydome_texture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, skydome_texture.data);

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

        // For each vertex in the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            // Extract vertices
            aiVector3D vertex = mesh->mVertices[j];
            skydome_vertices.push_back(vertex.x);
            skydome_vertices.push_back(vertex.y);
            skydome_vertices.push_back(vertex.z);

            // Extract texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                skydome_textures.push_back(texCoord.y);
                skydome_textures.push_back(texCoord.x);
            }
        }

        // For each face in the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace &face = mesh->mFaces[j];

            // Assume triangular faces
            if (face.mNumIndices == 3)
            {
                skydome_indices.push_back(face.mIndices[0]);
                skydome_indices.push_back(face.mIndices[1]);
                skydome_indices.push_back(face.mIndices[2]);
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
    glBufferData(GL_ARRAY_BUFFER, skydome_vertices.size() * sizeof(float), skydome_vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the texture coordinate buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SKYDOME].tbo);
    glBufferData(GL_ARRAY_BUFFER, skydome_textures.size() * sizeof(float), skydome_textures.data(), GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Bind and fill the index buffer object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[SKYDOME].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skydome_indices.size() * sizeof(unsigned int), skydome_indices.data(), GL_STATIC_DRAW);

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

    // Create vertex data for the quad
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
    // Specify vertex pointer location
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

    // Create vertex data for the quad
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
    // Specify vertex pointer location
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

    // Allocate space for 7 objects (Mesh, Splashscreen, Canvas, Sketch, Skydome, Sun, Moon)
    objects.resize(7);

    // Generate the vertex array objects; we need 2 objects: MESH and ORBIT
    this->initializeOrbit();
    this->initializeSplashscreen();
    this->initializeCanvas();
    this->initializeSkydome();

    // Set the glut timer callback for the sun animaton
    glutTimerFunc(100, Renderer::timerCallback, 0);

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
    this->time += 0.2f;
    if (this->time > 360.0f)
        this->time -= 360.0f;
}

void Renderer::sketch(float x, float y)
{
    // current page is used as index for the sketch_vertices, sketch_colors and sketch_indices arrays and "-1" removes the case of the landing screen
    short current_canvas = current_menu_page - 1;
    sketch_vertices[current_canvas].emplace_back(x);
    sketch_vertices[current_canvas].emplace_back(y);
    // The sketch must be drawn together with the canvas; to ensure
    // that the depth buffer is updated correctly, the sketch is drawn with a non 0 z-coordinate
    sketch_vertices[current_canvas].emplace_back(0.5);

    // If the current canvas is 0, set color to dark green, if 1 set color to brown, if 2 set color to light blue, if 3 set color dark blue
    if (current_canvas == RIDGES)
    {
        sketch_colors[current_canvas].emplace_back(0);
        sketch_colors[current_canvas].emplace_back(0.5);
        sketch_colors[current_canvas].emplace_back(0);
    }
    else if (current_canvas == PEAKS)
    {
        sketch_colors[current_canvas].emplace_back(0.5);
        sketch_colors[current_canvas].emplace_back(0.25);
        sketch_colors[current_canvas].emplace_back(0);
    }
    else if (current_canvas == RIVERS)
    {
        sketch_colors[current_canvas].emplace_back(0.3);
        sketch_colors[current_canvas].emplace_back(0.5);
        sketch_colors[current_canvas].emplace_back(0.8);
    }
    else if (current_canvas == BASINS)
    {
        sketch_colors[current_canvas].emplace_back(0);
        sketch_colors[current_canvas].emplace_back(0);
        sketch_colors[current_canvas].emplace_back(0.7);
    }

    if (x != 0xFFFFFFFFu)
        sketch_indices[current_canvas].emplace_back(sketch_vertices[current_canvas].size() / 3 - 1);
    else
        sketch_indices[current_canvas].emplace_back(0xFFFFFFFFu);
}

void Renderer::resetSketches()
{
    // current page is used as index for the sketch_vertices, sketch_colors and sketch_indices arrays and "-1" removes the case of the landing screen
    for (short current_canvas = 0; current_canvas < 4; current_canvas++)
    {
        sketch_vertices[current_canvas].clear();
        sketch_colors[current_canvas].clear();
        sketch_indices[current_canvas].clear();
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
    
    // Enable two vertex arrays: co-ordinates and color.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    
    // GLfloat materialDiffuse[] = {0.8f, 0.7f, 0.6f, 1.0f}; // Warm color for diffuse reflection
    // glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    
    glEnable(GL_PRIMITIVE_RESTART);                                                       // Enable primitive restart
    glDrawElements(GL_TRIANGLE_STRIP, instance->mesh_indices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);                                                      // Disable primitive restart
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    
    // Unbind the vertex array object and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::drawOrbit()
{
    // Draw the sun
    glPushMatrix();
        glRotatef(instance->time, 0, 0, 1);

        // Bind the sun texture
        glBindTexture(GL_TEXTURE_2D, instance->objects[SUN].texture);

        // Draw the sun
        glBindVertexArray(instance->objects[SUN].vao);

        // Enable two vertex arrays: co-ordinates and color.
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDrawElements(GL_TRIANGLE_STRIP, instance->sun_indices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        // Unbind the vertex array object and texture
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();

    // Draw the moon
    glPushMatrix();
        glMatrixMode(GL_MODELVIEW);

        glRotatef(instance->time, 0, 0, 1);

        // Bind the sun texture
        glBindTexture(GL_TEXTURE_2D, instance->objects[MOON].texture);
        
        // Draw the sun
        glBindVertexArray(instance->objects[MOON].vao);

        // Enable two vertex arrays: co-ordinates and color.
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDrawElements(GL_TRIANGLE_STRIP, instance->moon_indices.size(), GL_UNSIGNED_INT, 0); // Draw the triangles

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        // Unbind the vertex array object and texture
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}

void Renderer::drawSkydome()
{
    // Bind the skydome texture
    glBindTexture(GL_TEXTURE_2D, instance->objects[SKYDOME].texture);

    // Bind the vertex array object for the skydome
    glBindVertexArray(instance->objects[SKYDOME].vao);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Draw the skydome
    glDrawElements(GL_TRIANGLES, instance->skydome_indices.size(), GL_UNSIGNED_INT, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // Unbind the vertex array object and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
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
        glBindTexture(GL_TEXTURE_2D, instance->objects[CANVAS].texture);
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
        vector<float> vertices(instance->sketch_vertices[current_canvas].size());
        for (int i = 0; i < instance->sketch_vertices[current_canvas].size(); i = i + 3)
        {
            vertices[i] = instance->sketch_vertices[current_canvas][i] * width;
            vertices[i + 1] = instance->sketch_vertices[current_canvas][i + 1] * height;
            vertices[i + 2] = instance->sketch_vertices[current_canvas][i + 2];
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
        glBufferData(GL_ARRAY_BUFFER, instance->sketch_vertices[current_canvas].size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
        // Set the vertex attribute pointer for positions
        glVertexPointer(3, GL_FLOAT, 0, vertices.data());

        // Bind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SKETCH].cbo);
        // Update the vertex buffer data for points
        glBufferData(GL_ARRAY_BUFFER, instance->sketch_colors[current_canvas].size() * sizeof(GLfloat), instance->sketch_colors[current_canvas].data(), GL_STATIC_DRAW);
        // Set the vertex attribute pointer for colors
        glColorPointer(3, GL_FLOAT, 0, instance->sketch_colors[current_canvas].data());

        // Bind the index buffer object
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->objects[SKETCH].ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, instance->sketch_indices[current_canvas].size() * sizeof(GLuint), instance->sketch_indices[current_canvas].data(), GL_STATIC_DRAW);

        // Enable primitive restart
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(0xFFFFFFFFu);

        // If the current page is rivers or ridges, draw lines
        if (current_canvas == RIDGES || current_canvas == RIVERS)
        {
            // Increment line width
            glLineWidth(5.0f);
            // Draw the sketch using indices
            glDrawElements(GL_LINE_STRIP, instance->sketch_indices[current_canvas].size(), GL_UNSIGNED_INT, instance->sketch_indices[current_canvas].data());
        }
        if (current_canvas == PEAKS || current_canvas == BASINS)
        {
            // Increment points size
            glPointSize(5.0f);
            // Draw the sketch using indices
            glDrawElements(GL_POINTS, instance->sketch_indices[current_canvas].size(), GL_UNSIGNED_INT, instance->sketch_indices[current_canvas].data());
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
    // Disable lighting
    glDisable(GL_LIGHTING);
    
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
            int total_seconds = static_cast<int>(instance->time * 24.0f * 60.0f * 60.0f / 360.0f);
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
    
    // Re-enable lighting
    glEnable(GL_LIGHTING);
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
        // Draw a text in the middle saying "Time"
        instance->drawMesh();
        instance->drawOrbit();
        instance->drawSkydome();
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
    // Draw the light source
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        glRotatef(instance->time, 0, 0, 1);
        GLfloat light_position[4];
        // Initialize the light position to the sun's position
        light_position[0] = 0;
        light_position[1] = 10000;
        light_position[2] = 3600;
        light_position[3] = 0.0f;
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glPopMatrix();
    glutSwapBuffers();
}