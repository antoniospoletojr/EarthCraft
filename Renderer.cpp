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
    
    // Allocate a terrain object
    terrain = new Terrain();
    terrain->initialize("./assets/sketches/heightmap.png", 16.0);
    
    // Retrieve the map
    Vec3<float> *map = terrain->getMap();
    
    // Print the map info
    terrain->getInfo();
    float max_y = terrain->getMaxHeight();
    
    // Get the dimension of the map useful for allocations
    int dim = terrain->getDim();
    float world_scale = terrain->getWorldScale();
    mesh_dim = dim*world_scale;
    
    //Reset mesh arrays if they are not empty
    mesh_vertices.clear();
    mesh_colors.clear();
    mesh_indices.clear();
    
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
            
            // Calculate the normalized height value
            float normalizedHeight = map[i * dim + j].y() / max_y;

            // Define the green and brown color values with adjusted components
            float greenR = 0.0f, greenG = 0.8f, greenB = 0.0f;
            float brownR = 0.7f, brownG = 0.4f, brownB = 0.1f;
            
            // Interpolate the colors based on the normalized height
            float red = greenR + (brownR - greenR) * normalizedHeight;
            float green = greenG + (brownG - greenG) * normalizedHeight;
            float blue = greenB + (brownB - greenB) * normalizedHeight;

            // Set the colors
            mesh_colors.emplace_back(red);
            mesh_colors.emplace_back(green);
            mesh_colors.emplace_back(blue);
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
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

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
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
            sun_vertices.push_back(vertex.y * 20 + 3000);
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
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        
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
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

void Renderer::initialize(Camera *camera)
{
    
    // Set the camera
    this->camera = camera;
    
    // Allocate space for 5 objects (Mesh, Sun, Splashscreen, Canvas, Sketch)
    objects.resize(5);
    
    // Generate the vertex array objects; we need 2 objects: MESH and SUN
    this->initializeSun();
    this->initializeSplashscreen();
    this->initializeCanvas();
    
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

void Renderer::sketch(float x, float y)
{
    // current page is used as index for the sketch_vertices, sketch_colors and sketch_indices arrays and "-1" removes the case of the landing screen
    short current_canvas = current_menu_page-1;
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
        sketch_indices[current_canvas].emplace_back(sketch_vertices[current_canvas].size()/3 - 1);
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
    
    instance->moveSun();
    glutPostRedisplay();
    glutTimerFunc(25, Renderer::timerCallback, 0);
}

void Renderer::drawMesh()
{
    // Draw the terrain
    glBindVertexArray(instance->objects[MESH].vao);
    // Enable two vertex arrays: co-ordinates and color.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_PRIMITIVE_RESTART);                                                         // Enable primitive restart
    glDrawElements(GL_QUAD_STRIP, instance->mesh_indices.size(), GL_UNSIGNED_INT, 0);       // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);                                                        // Disable primitive restart
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    // Unbind the VAO
    glBindVertexArray(0);
}

void Renderer::drawSun()
{
    // Draw the sun
    glBindVertexArray(instance->objects[SUN].vao);
    // Enable two vertex arrays: co-ordinates and color.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_PRIMITIVE_RESTART);                                                         // Enable primitive restart
    glDrawElements(GL_TRIANGLE_STRIP, instance->sun_indices.size(), GL_UNSIGNED_INT, 0);    // Draw the triangles
    glDisable(GL_PRIMITIVE_RESTART);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    // Unbind the VAO
    glBindVertexArray(0);
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
        instance->drawMesh();
        instance->drawSun();
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
