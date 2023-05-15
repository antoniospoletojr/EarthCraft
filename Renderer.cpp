#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "random"
#include "Terrain.h"
#include "Renderer.h"

#define VERTICES 0
#define INDICES 1

#define MESH 0
#define SUN 1
#define PLANET 2

using namespace std;

Renderer* Renderer::instance = nullptr;

// Default constructor
Renderer::Renderer()
{
    if (Renderer::instance == nullptr)
        Renderer::instance = this;
}

// Destructor
Renderer::~Renderer()
{
    // Delete the vertex buffer objects
    glDeleteBuffers(2, vbo);

    // Disable the vertex arrays
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    // Unbind any buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Deallocate memory
    mesh_vertices.clear();
    mesh_colors.clear();
    mesh_indices.clear();
    
    Renderer::instance = nullptr;
}

void Renderer::initializeMesh(Terrain *terrain)
{
    // Bind the vertex array object for the mesh
    glBindVertexArray(vao[MESH]); 

    // Generate the vertex buffer objects
    glGenBuffers(2, vbo);
    
    // Enable the vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    // Use maximum unsigned int as restart index
    glPrimitiveRestartIndex(0xFFFFFFFFu);
    
    // Retrieve the map
    Vec3<float> *map = terrain->getMap();

    // Print the map info
    terrain->getInfo();

    // Get the dimension of the map useful for allocations
    int dim = terrain->getDim();
    
    // Allocate memory for the vertices and colors
    mesh_vertices.reserve(dim * dim * 3);
    mesh_colors.reserve(dim * dim * 3);
    
    // Generate vertices and colors
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            mesh_vertices.emplace_back(map[i * dim + j].x());
            mesh_vertices.emplace_back(map[i * dim + j].y());
            mesh_vertices.emplace_back(map[i * dim + j].z());
            
            // Set the colors to be proportional to the height of the terrain and range from green to brown
            mesh_colors.emplace_back(map[i * dim + j].y() / 255.0);
            mesh_colors.emplace_back(map[i * dim + j].y() / 255.0);
            mesh_colors.emplace_back(255.0);
        }
    }
    
    // Generate indices for triangle strips
    for (int z = 0; z < dim - 1; z++)
    {
        // Start a new strip
        mesh_indices.emplace_back(z * dim);
        for (int x = 0; x < dim; x++)
        {
            // Add vertices to strip
            mesh_indices.emplace_back(z * dim + x);
            mesh_indices.emplace_back((z + 1) * dim + x);
        }
        // Use primitive restart to start a new strip
        mesh_indices.emplace_back(0xFFFFFFFFu);
    }
    
    // Bind the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTICES]);
    // Reserve space for the vertex buffer
    glBufferData(GL_ARRAY_BUFFER, (mesh_vertices.size()+mesh_colors.size())*sizeof(float), NULL, GL_STATIC_DRAW);
    // Copy vertex coordinates data into first half of vertex buffer.
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh_vertices.size()*sizeof(float), mesh_vertices.data());
    // Copy vertex color data into second half of vertex buffer.
    glBufferSubData(GL_ARRAY_BUFFER, mesh_vertices.size()*sizeof(float), mesh_colors.size()*sizeof(float), mesh_colors.data());
    
    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_indices.size()*sizeof(GLuint), mesh_indices.data(), GL_STATIC_DRAW);
    
    // Specify vertex and color pointers to the start of the respective data
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColorPointer(3, GL_FLOAT, 0, (GLvoid*)(mesh_vertices.size()*sizeof(float)));
}

void Renderer::initializeSun()
{
    glBindVertexArray(vao[SUN]);
    glGenBuffers(2, vbo);
    
    Assimp::Importer importer;

    // Load the .obj file
    const aiScene* scene = importer.ReadFile("sun.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
     // Check if the scene was loaded successfully
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        std::cout << "Error loading scene: " << importer.GetErrorString() << std::endl;
    
    cout << "Number of meshes: " << scene->mNumMeshes << endl;
    
    // For each mesh in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        
        // For each vertex in the mesh
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            // Extract vertices
            aiVector3D vertex = mesh->mVertices[j];
            sun_vertices.push_back(vertex.x * 20);
            sun_vertices.push_back(vertex.y * 20 + 1000);
            sun_vertices.push_back(vertex.z * 20);
            
            // Extract colors
            if (mesh->HasVertexColors(0))
            {
                printf("Has colors\n");
                aiColor4D color = mesh->mColors[0][j];
                sun_colors.push_back(color.r);
                sun_colors.push_back(color.g);
                sun_colors.push_back(color.b);
                sun_colors.push_back(color.a);
            }
            else
            {   // Set color to yellow
                sun_colors.push_back(0.0f);
                sun_colors.push_back(0.0f);
                sun_colors.push_back(1.0f);
                sun_colors.push_back(1.0f);
            }
        }
        
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace& face = mesh->mFaces[j];
            
            for (unsigned int k = 0; k < face.mNumIndices; ++k)
            {
                sun_indices.push_back(face.mIndices[k]);
            }
        }
    }
    
    // Print the number of vertices and indices
    std::cout << "Number of vertices: " << sun_indices.size() / 8 << std::endl;
    std::cout << "Number of indices: " << sun_indices.size() << std::endl;
    
    // Enable two vertex arrays: co-ordinates and color.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, (sun_vertices.size() + sun_colors.size()) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sun_vertices.size()*sizeof(float), sun_vertices.data());
    glBufferSubData(GL_ARRAY_BUFFER, sun_vertices.size()*sizeof(float), sun_colors.size() * sizeof(float), sun_colors.data());
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sun_indices.size() * sizeof(GLuint), &sun_indices[0], GL_STATIC_DRAW);
    
    // Specify vertex and color pointers to the start of the respective data
    glVertexPointer(3, GL_FLOAT, 0, 0); 
    glColorPointer(3, GL_FLOAT, 0, (GLvoid*)(sun_vertices.size()*sizeof(float))); 
}

void Renderer::initialize(Terrain *terrain)
{
    glutTimerFunc(100, Renderer::timerCallback, 0);
    // Generate the vertex array objects; we need 2 objects: MESH and SUN
    glGenVertexArrays(3, vao);
    this->initializeMesh(terrain);
    this->initializeSun();
}

void Renderer::timerCallback(int value)
{  
    glutTimerFunc(1000, Renderer::timerCallback, 0);
    instance->moveSun();
    glutPostRedisplay();
}

void Renderer::moveSun()
{
    glBindVertexArray(vao[SUN]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTICES]);
    
    // Map the buffer object
    GLfloat* vertices = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    
    // Iterate through each vertex and update its x-coordinate
    for (int i = 0; i < sun_vertices.size(); i+=3)
        vertices[i] += 10;
    
    // Unmap the buffer object
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Unbind the VAO
    glBindVertexArray(0);
}

void Renderer::draw()
{    
    glBindVertexArray(vao[MESH]);
    glEnable(GL_PRIMITIVE_RESTART);                                          // Enable primitive restart
    glDrawElements(GL_QUAD_STRIP, mesh_indices.size(), GL_UNSIGNED_INT, 0);  // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);                                         // Disable primitive restart
    
    glBindVertexArray(vao[SUN]);
    glEnable(GL_PRIMITIVE_RESTART);                                         // Enable primitive restart
    glDrawElements(GL_TRIANGLE_STRIP, sun_indices.size(), GL_UNSIGNED_INT, 0);  // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);
}
