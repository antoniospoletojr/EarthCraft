#include <GL/glew.h>
#include "random"
#include "Terrain.h"
#include "Mesh.h"

#define VERTICES 0
#define INDICES 1

// Default constructor
Mesh::Mesh(){}

// Destructor
Mesh::~Mesh()
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
    vertices.clear();
    colors.clear();
    indices.clear();
}

void Mesh::initialize(Terrain *terrain)
{
    // Enable the vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    // Generate the vertex buffer objects
    glGenBuffers(2, vbo);

    // Use maximum unsigned int as restart index
    glPrimitiveRestartIndex(0xFFFFFFFFu);

    // Retrieve the map
    Vec3<float> *map = terrain->getMap();

    // Retrieve the map info
    terrain->getInfo();

    // Get the dimension of the map useful for allocations
    int dim = terrain->getDim();

    // Allocate memory for the vertices and colors
    vertices.reserve(dim * dim * 3);
    colors.reserve(dim * dim * 3);
    
    // Generate vertices and colors
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            vertices.emplace_back(map[i * dim + j].x());
            vertices.emplace_back(map[i * dim + j].y());
            vertices.emplace_back(map[i * dim + j].z());
            
            // Set the colors to be proportional to the height of the terrain and range from brown to white
            colors.emplace_back(map[i * dim + j].y() / 255.0);
            colors.emplace_back(map[i * dim + j].y() / 255.0);
            colors.emplace_back(255.0);
        }
    }
    
    // Generate indices for triangle strips
    for (int z = 0; z < dim - 1; z++)
    {
        // Start a new strip
        indices.emplace_back(z * dim);
        for (int x = 0; x < dim; x++)
        {
            // Add vertices to strip
            indices.emplace_back(z * dim + x);
            indices.emplace_back((z + 1) * dim + x);
        }
        // Use primitive restart to start a new strip
        indices.emplace_back(0xFFFFFFFFu);
    }
    
    // Bind the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTICES]);
    // Reserve space for the vertex buffer
    glBufferData(GL_ARRAY_BUFFER, (vertices.size()+colors.size())*sizeof(float), NULL, GL_STATIC_DRAW);
    // Copy vertex coordinates data into first half of vertex buffer.
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size()*sizeof(float), vertices.data());
    // Copy vertex color data into second half of vertex buffer.
    glBufferSubData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), colors.size()*sizeof(float), colors.data());
    
    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    
    // Specify vertex and color pointers to the start of the respective data
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColorPointer(3, GL_FLOAT, 0, (GLvoid*)(vertices.size()*sizeof(float)));
}

void Mesh::draw()
{    
    glEnable(GL_PRIMITIVE_RESTART);                                         // Enable primitive restart
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);  // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);                                        // Disable primitive restart
}
