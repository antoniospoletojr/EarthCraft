#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "random"
#include "Terrain.h"
#include "Renderer.h"
#include "Constants.h"


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
    // Disable the vertex arrays
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Unbind any buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // Deallocate memory
    mesh_vertices.clear();
    mesh_colors.clear();
    mesh_indices.clear();
    sun_vertices.clear();
    sun_colors.clear();
    sun_indices.clear();
    sketch_vertices.clear();
    objects.clear();
    
    // Delete the vertex array objects
    glDeleteVertexArrays(1, &objects[MESH].vao);
    glDeleteVertexArrays(1, &objects[SUN].vao);
    glDeleteVertexArrays(1, &objects[SPLASHSCREEN].vao);
    glDeleteVertexArrays(1, &objects[CANVAS].vao);
    
    // Deallocate opencv objects
    splashscreen.release();
    canvas.release();
    splashscreen_frame.release();
    canvas_frame.release();
    
    Renderer::instance = nullptr;
}

void Renderer::initializeMesh()
{
    // Generate the vertex array object for the mesh
    glGenVertexArrays(1, &objects[MESH].vao);
    // Bind the vertex array object for the mesh
    glBindVertexArray(objects[MESH].vao);
    
    // Generate the buffer objects
    glGenBuffers(1, &objects[MESH].vbo);
    glGenBuffers(1, &objects[MESH].cbo);
    glGenBuffers(1, &objects[MESH].ibo);
    
    // Retrieve the map
    Vec3<float> *map = terrain->getMap();
    
    // Print the map info
    terrain->getInfo();
    
    // Get the dimension of the map useful for allocations
    int dim = terrain->getDim();
    float world_scale = terrain->getWorldScale();
    mesh_dim = dim*world_scale;
    
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
    
    // Enable the vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Use maximum unsigned int as restart index
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFFu);

    // Bind and fill the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MESH].vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_vertices.size() * sizeof(float), mesh_vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the color buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[MESH].cbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_colors.size() * sizeof(float), mesh_colors.data(), GL_STATIC_DRAW);
    glColorPointer(3, GL_FLOAT, 0, 0);
    
    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[MESH].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_indices.size()*sizeof(GLuint), mesh_indices.data(), GL_STATIC_DRAW);
    
    // Unbind everything
    glBindVertexArray(0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void Renderer::initializeSun()
{
    // Generate the vertex array object for the sun
    glGenVertexArrays(1, &objects[SUN].vao);
    // Bind the vertex array object for the sun
    glBindVertexArray(objects[SUN].vao);

    // Generate the buffer objects
    glGenBuffers(1, &objects[SUN].vbo);
    glGenBuffers(1, &objects[SUN].cbo);
    glGenBuffers(1, &objects[SUN].ibo);
    
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

    // Bind and fill the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SUN].vbo);
    glBufferData(GL_ARRAY_BUFFER, sun_vertices.size() * sizeof(float), sun_vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill the color buffer object
    glBindBuffer(GL_ARRAY_BUFFER, objects[SUN].cbo);
    glBufferData(GL_ARRAY_BUFFER, sun_colors.size() * sizeof(float), sun_colors.data(), GL_STATIC_DRAW);
    glColorPointer(3, GL_FLOAT, 0, 0);

    // Bind and fill indices buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[SUN].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sun_indices.size() * sizeof(GLuint), sun_indices.data(), GL_STATIC_DRAW);

    // Unbind everything
    glBindVertexArray(0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void Renderer::initializeSplashscreen()
{
    // Load the splashscreen video and the first frame
    instance->splashscreen.open("splashscreen.mp4");
    instance->splashscreen.read(instance->splashscreen_frame);
    
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
    std::vector<GLfloat> colors = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
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
    // Load the splashscreen video and the first frame
    instance->canvas.open("canvas.mp4");
    instance->canvas.read(instance->canvas_frame);

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

void Renderer::initializeSketch()
{
    // Generate the vertex array object for the sketch
    // glGenVertexArrays(1, &objects[SKETCH].vao);
    // // Bind the vertex array object for the sketch
    // glBindVertexArray(objects[SKETCH].vao);

    // Generate the buffer objects
    // glGenBuffers(1, &objects[SKETCH].vbo);
    // glGenBuffers(1, &objects[SKETCH].tbo);
    // glGenBuffers(1, &objects[SKETCH].cbo);
    //glGenBuffers(1, &objects[SKETCH].ibo);

    // glEnableClientState(GL_VERTEX_ARRAY);
    // glEnableClientState(GL_COLOR_ARRAY);
    
    // // Bind the vertex buffer object
    // glBindBuffer(GL_ARRAY_BUFFER, objects[SKETCH].vbo);
    // glVertexPointer(3, GL_FLOAT, 0, 0);
    
    // // Bind the color buffer object
    // glBindBuffer(GL_ARRAY_BUFFER, objects[SKETCH].cbo);
    // glColorPointer(3, GL_FLOAT, 0, 0);

    // Bind the index buffer object
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->objects[SKETCH].ibo);

    // Unbind everything
    // glBindVertexArray(0);
    // glDisableClientState(GL_VERTEX_ARRAY);
    // glDisableClientState(GL_COLOR_ARRAY);
}

void Renderer::initialize(Terrain *terrain, Camera *camera)
{
    // Set the terrain
    this->terrain = terrain;
    
    // Set the camera
    this->camera = camera;
    
    // Allocate space for 5 objects (Mesh, Sun, Splashscreen, Canvas, Sketch)
    objects.resize(5);

    // Generate the vertex array objects; we need 2 objects: MESH and SUN
    this->initializeMesh();
    this->initializeSun();
    this->initializeSplashscreen();
    this->initializeCanvas();
    
    // Set the glut timer callback for the sun animaton
    glutTimerFunc(100, Renderer::timerCallback, 0);
    
    glutDisplayFunc(Renderer::draw);
}

void Renderer::moveSun()
{
    glBindVertexArray(objects[SUN].vao);
    glBindBuffer(GL_ARRAY_BUFFER, objects[SUN].vbo);

    // Map the buffer object
    GLfloat* vertices = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    
    // Iterate through each vertex and update its x-coordinate
    bool has_reached_end = false;
    const int numVertices = sun_vertices.size();

    for (int i = 0; i < numVertices; i += 3)
    {
        vertices[i] += 1;
        if (vertices[i] > mesh_dim / 2 + 1)
            has_reached_end = true;
    }

    if (has_reached_end)
    {
        for (int i = 0; i < numVertices; i += 3)
        {
            vertices[i] -= mesh_dim;
        }
    }

    // Unmap the buffer object
    glUnmapBuffer(GL_ARRAY_BUFFER);
    
    // Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Unbind the VAO
    glBindVertexArray(0);
}

void Renderer::incrementMenuPage()
{
    current_menu_page++;
    // If the current page is the last one, go back to the first one
    if (current_menu_page > 4)
        current_menu_page = -1;
}

short Renderer::getCurrentMenuPage()
{
    return current_menu_page;
}

void Renderer::sketch(float x, float y)
{
    sketch_vertices.emplace_back(x);
    sketch_vertices.emplace_back(y);
    // The sketch must be drawn together with the canvas; to ensure 
    // that the depth buffer is updated correctly, the sketch is drawn with a non 0 z-coordinate
    sketch_vertices.emplace_back(0.5);
    
    // Set the color to brown
    sketch_colors.emplace_back(0.6f);
    sketch_colors.emplace_back(0.3f); 
    sketch_colors.emplace_back(0.0f);

    static GLuint counter;
    if (x != 0xFFFFFFFFu)
    {
        sketch_indices.emplace_back(counter);
        counter = counter + 1;
    }
    else
    {
        sketch_indices.emplace_back(0xFFFFFFFFu);
        counter = counter + 1;
    }
}

void Renderer::timerCallback(int value)
{
    switch (instance->getCurrentMenuPage())
    {
        case LANDIND_SCREEN:
            if (!instance->splashscreen.read(instance->splashscreen_frame))
            {
                instance->splashscreen.set(cv::CAP_PROP_POS_FRAMES, 0);
                instance->splashscreen.read(instance->splashscreen_frame);
            }
            break;
        case RIDGES_SCREEN:
            if (!instance->canvas.read(instance->canvas_frame))
            {
                instance->canvas.set(cv::CAP_PROP_POS_FRAMES, 0);
                instance->canvas.read(instance->canvas_frame);
            }
            break;
        case PEAKS_SCREEN:
            break;
        case RIVERS_SCREEN:
            break;
        case BASINS_SCREEN:
            break;
        default:
            break;
    }
    
    instance->moveSun();
    glutPostRedisplay();
    glutTimerFunc(25, Renderer::timerCallback, 0);
}

void Renderer::drawMesh()
{
    // Draw the terrain
    glBindVertexArray(instance->objects[MESH].vao);
    glEnable(GL_PRIMITIVE_RESTART);                                                         // Enable primitive restart
    glDrawElements(GL_QUAD_STRIP, instance->mesh_indices.size(), GL_UNSIGNED_INT, 0);       // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);                                                        // Disable primitive restart
    glBindVertexArray(0);
}

void Renderer::drawSun()
{
    // Draw the sun
    glBindVertexArray(instance->objects[SUN].vao);
    glEnable(GL_PRIMITIVE_RESTART);                                                         // Enable primitive restart
    glDrawElements(GL_TRIANGLE_STRIP, instance->sun_indices.size(), GL_UNSIGNED_INT, 0);    // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);
    // Unbind the VAO
    glBindVertexArray(0);
}

void Renderer::drawSplashscreen()
{
    if (!instance->splashscreen_frame.empty())
    {           
        // Save the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        
        // Set the projection matrix to orthographic
        float width = glutGet(GLUT_WINDOW_WIDTH);
        float height = glutGet(GLUT_WINDOW_HEIGHT);
        glLoadIdentity();
        glOrtho(-width/2, width/2, -height/2, height/2, -1, 1);
        
        // Update texture data
        glBindTexture(GL_TEXTURE_2D, instance->objects[SPLASHSCREEN].texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, instance->splashscreen_frame.cols, instance->splashscreen_frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, instance->splashscreen_frame.data);

        // Update width and height values in a single line
        std::vector<GLfloat> vertices = {-width / 2, -height / 2, width / 2, -height / 2, width / 2, height / 2, -width / 2, height / 2};
        // Bind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SPLASHSCREEN].vbo);
        // Update the vertex buffer data
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(GLfloat), vertices.data());

        // Render the splash screen
        glBindVertexArray(instance->objects[SPLASHSCREEN].vao);
        glDrawArrays(GL_QUADS, 0, 4);
        glBindVertexArray(0);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // Restore the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
}

void Renderer::drawCanvas()
{
    if (!instance->canvas_frame.empty())
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, instance->canvas_frame.cols, instance->canvas_frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, instance->canvas_frame.data);
        
        // Update width and height values in a single line
        std::vector<GLfloat> vertices = {0, 0, width, 0, width, height, 0, height};
        

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
        
        // Restore the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
}

void Renderer::drawSketch()
{
    if (!instance->canvas_frame.empty())
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
        
        vector<float> vertices(instance->sketch_vertices.size());
        for (int i = 0; i < instance->sketch_vertices.size(); i = i + 3)
        {
                vertices[i] = instance->sketch_vertices[i] * width;
                vertices[i + 1] = instance->sketch_vertices[i + 1] * height;
                vertices[i + 2] = instance->sketch_vertices[i + 2];
        }
        
        // Render the sketch
        glBindVertexArray(instance->objects[SKETCH].vao);

        // Enable the vertex arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Bind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SKETCH].vbo);
        // Update the vertex buffer data for points
        glBufferData(GL_ARRAY_BUFFER, instance->sketch_vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
        // Set the vertex attribute pointer for positions
        glVertexPointer(3, GL_FLOAT, 0, vertices.data());
        
        // Bind the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, instance->objects[SKETCH].cbo);
        // Update the vertex buffer data for points
        glBufferData(GL_ARRAY_BUFFER, instance->sketch_colors.size() * sizeof(GLfloat), instance->sketch_colors.data(), GL_STATIC_DRAW);
        // Set the vertex attribute pointer for colors
        glColorPointer(3, GL_FLOAT, 0, instance->sketch_colors.data());
        
        // Bind the index buffer object
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->objects[SKETCH].ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, instance->sketch_indices.size() * sizeof(GLuint), instance->sketch_indices.data(), GL_STATIC_DRAW);

        // Increment line width
        glLineWidth(3.0f);
        
        // Enable primitive restart
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(0xFFFFFFFFu);
        // Draw the sketch using indices
        glDrawElements(GL_LINE_STRIP, instance->sketch_indices.size(), GL_UNSIGNED_INT, instance->sketch_indices.data());
        glDisable(GL_PRIMITIVE_RESTART);
        
        // Deallocate memory
        vertices.clear();
        
        // Restore the previous projection matrix
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
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
    
    // Switch on the current menu page
    switch (instance->getCurrentMenuPage())
    {
        case 0:
            instance->drawSplashscreen();
            break;
        case 1:
            instance->drawCanvas();
            instance->drawSketch();
            break;
        case 2:
            instance->drawSketch();
            break;
        case 3:
            //instance->drawCanvas();
            break;
        case 4:
            //instance->drawCanvas();
            break;
        case -1:
            instance->drawMesh();
            instance->drawSun();
            break;
        default:
            break;
    }

    glutSwapBuffers();
}
