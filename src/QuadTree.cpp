/**
@file
@brief QuadTree source file.
*/

#include "QuadTree.h"


QuadNode::QuadNode(QuadTree *quadtree, Terrain* terrain, float x, float width, float z, float depth)
{
    this->quadtree = quadtree;
    this->x = x;
    this->width = width;
    this->z = z;
    this->depth = depth;

    int delta_x = width - x;
    int delta_z = depth - z;
    int dim = terrain->getDim();
    Vec3<float> *map = terrain->getHeightmap();
           
    // Calculate the world coordinates corners of the quadnode
    this->top_left_corner.u = map[(int)x*dim+(int)z].x;
    this->top_left_corner.v = map[(int)x*dim+(int)z].z;

    this->top_right_corner.u = map[((int)x+delta_x - 1)*dim + (int)z].x;
    this->top_right_corner.v = map[((int)x+delta_x - 1)*dim + (int)z].z;
    
    this->bottom_left_corner.u = map[(int)x*dim + (int)z+delta_z - 1].x;
    this->bottom_left_corner.v = map[(int)x*dim + (int)z+delta_z - 1].z;
    
    this->bottom_right_corner.u = map[((int)x+delta_x - 1)*dim + (int)z+delta_z - 1].x;
    this->bottom_right_corner.v = map[((int)x+delta_x - 1)*dim + (int)z+delta_z - 1].z;
    
    // Calculate node diagonal length
    float diag = sqrt(pow(delta_x, 2) + pow(delta_z, 2));
    
    // If the node is too big, subdivide it
    if(diag > CHUNK_SIZE)
    {
        // Fill the 4 children with subportion of the terrain map
        this->NW_child = new QuadNode(quadtree, terrain, x, ceil((width + x) / 2.0f), z, ceil((depth + z) / 2.0f));
        this->NE_child = new QuadNode(quadtree, terrain, floor((width + x) / 2.0f), width, z, ceil((depth + z) / 2.0f));
        this->SW_child = new QuadNode(quadtree, terrain, x, ceil((width + x) / 2.0f), floor((depth + z) / 2.0f), depth);
        this->SE_child = new QuadNode(quadtree, terrain, floor((width + x) / 2.0f), width, floor((depth + z) / 2.0f), depth);
    }
    else // this node is a leaf
    {        
        // Reset mesh arrays if they are not empty
        this->object.vertices.clear();
        this->object.indices.clear();
        this->object.textures.clear();
        this->object.normals.clear();
        
        // Generate vertices, textures and nomals values for the mesh
        for (int i = x; i < x+delta_x; i++)
        {
            for (int j = z; j < z+delta_z; j++)
            {
                this->object.vertices.push_back(map[i * dim + j].x);
                this->object.vertices.push_back(map[i * dim + j].y);
                this->object.vertices.push_back(map[i * dim + j].z);
                
                this->object.textures.push_back((float)i / dim);
                this->object.textures.push_back((float)j / dim);
                
                this->object.normals.push_back(0.0f);
                this->object.normals.push_back(0.0f);
                this->object.normals.push_back(0.0f);
            }
        }

        // Generate indices for the mesh        
        for (int j = 0; j < delta_z-1; j++)
        {
            // Start a new strip
            this->object.indices.push_back(j * delta_z);
            for (int i = 0; i < delta_x; i++)
            {
                // Add vertices to strip
                this->object.indices.push_back((j + 1) * delta_z + i);
                this->object.indices.push_back(j * delta_z + i);
            }
            // Use primitive restart to start a new strip
            this->object.indices.push_back(0xFFFFFFFFu);
        }
        
        // Calculate normals
        for (int i = 0; i < this->object.indices.size()-3; i += 2)
        {    
            if (this->object.indices[i+1] == 0xFFFFFFFFu)
                continue;
            
            // Get the indices of the triangle vertices
            int i1 = this->object.indices[i];
            int i2 = this->object.indices[i + 1];
            int i3 = this->object.indices[i + 2];
            
            // Get the vertices of the triangle into Vec3 objects
            Vec3<float> v1;
            v1.x = this->object.vertices[i1 * 3];
            v1.y = this->object.vertices[i1 * 3 + 1];
            v1.z = this->object.vertices[i1 * 3 + 2];
            
            Vec3<float> v2;
            v2.x = this->object.vertices[i2 * 3];
            v2.y = this->object.vertices[i2 * 3 + 1];
            v2.z = this->object.vertices[i2 * 3 + 2];
            
            Vec3<float> v3;
            v3.x = this->object.vertices[i3 * 3];
            v3.y = this->object.vertices[i3 * 3 + 1];
            v3.z = this->object.vertices[i3 * 3 + 2];
            
            // Get 2 edges of the triangle
            Vec3<float> u1 = subtract(v2, v1);
            Vec3<float> u2 = subtract(v3, v1);
            
            // Calculate the normal of the triangle
            Vec3<float> normal = crossProduct(u1, u2);
            
            // Add the normal to the normals array
            this->object.normals[i1 * 3] += normal.x;
            this->object.normals[i1 * 3 + 1] += normal.y;
            this->object.normals[i1 * 3 + 2] += normal.z;
            
            this->object.normals[i2 * 3] += normal.x;
            this->object.normals[i2 * 3 + 1] += normal.y;
            this->object.normals[i2 * 3 + 2] += normal.z;
            
            this->object.normals[i3 * 3] += normal.x;
            this->object.normals[i3 * 3 + 1] += normal.y;
            this->object.normals[i3 * 3 + 2] += normal.z;
        }
       
        // Generate the vertex array object for the mesh
        glGenVertexArrays(1, &this->object.vao);
        // Bind the vertex array object for the mesh
        glBindVertexArray(this->object.vao);
        
        // Generate the buffer objects
        glGenBuffers(1, &this->object.vbo);
        glGenBuffers(1, &this->object.tbo);
        glGenBuffers(1, &this->object.ibo);
        glGenBuffers(1, &this->object.nbo);
        
        // Use maximum unsigned int as restart index
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(0xFFFFFFFFu);
        
        // Bind and fill the vertex buffer object
        glBindBuffer(GL_ARRAY_BUFFER, this->object.vbo);
        glBufferData(GL_ARRAY_BUFFER, this->object.vertices.size() * sizeof(float), this->object.vertices.data(), GL_STATIC_DRAW);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        
        // Bind and fill the texture coordinate buffer object
        glBindBuffer(GL_ARRAY_BUFFER, this->object.tbo);
        glBufferData(GL_ARRAY_BUFFER, this->object.textures.size() * sizeof(float), this->object.textures.data(), GL_STATIC_DRAW);
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
        
        // Bind and fill the normals buffer object
        glBindBuffer(GL_ARRAY_BUFFER, this->object.nbo);
        glBufferData(GL_ARRAY_BUFFER, this->object.normals.size() * sizeof(float), this->object.normals.data(), GL_STATIC_DRAW);
        glNormalPointer(GL_FLOAT, 0, 0);
        
        // Bind and fill indices buffer.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->object.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->object.indices.size() * sizeof(GLuint), this->object.indices.data(), GL_STATIC_DRAW);
        
        // Unbind everything
        glBindVertexArray(0);
        
        // Fill the 4 children with nullptr
        this->NW_child = nullptr;
        this->NE_child = nullptr;
        this->SW_child = nullptr;
        this->SE_child = nullptr;
    }
}

QuadNode::~QuadNode()
{
    delete this->NW_child;
    delete this->NE_child;
    delete this->SW_child;
    delete this->SE_child;
}

void QuadNode::draw()
{
    // If the node is a leaf
    if (this->NW_child == nullptr && this->NE_child == nullptr && this->SW_child == nullptr && this->SE_child == nullptr)
    {   
        // If it is inside the view frustum
        if(this->quadtree->isInFrustum(this))
        {
            // Draw the terrain
            glBindVertexArray(this->object.vao);

            // Enable two vertex arrays: co-ordinates and color.
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            
            glEnable(GL_PRIMITIVE_RESTART);
            glDrawElements(GL_TRIANGLE_STRIP, this->object.indices.size(), GL_UNSIGNED_INT, 0);
            glDisable(GL_PRIMITIVE_RESTART);
            
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);

            glBindVertexArray(0);
        }
    }
    else // The node is not a leaf
    {
        if(this->quadtree->isInFrustum(this))
        {
            this->NW_child->draw();
            this->NE_child->draw();
            this->SW_child->draw();
            this->SE_child->draw();
        }
    }
}

QuadTree::QuadTree()
{
    this->root = nullptr;
    this->fov_angle = cos(FOV_ANGLE * M_PI / 180.0f);
}


QuadTree::~QuadTree()
{
    delete this->root;
}

void QuadTree::initialize(Terrain *terrain)
{
    printf("Building quadtree...\n");

    // Load the mesh texture image
    cv::Mat mesh_texture = terrain->getTexture();
    
    // Generate and bind a texture object
    glGenTextures(1, &this->texture_id);
    glBindTexture(GL_TEXTURE_2D, this->texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload the texture image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mesh_texture.cols, mesh_texture.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, mesh_texture.data);
    
    // Get the dimension of the map, useful for allocations
    int dim = terrain->getDim();
    
    // Initialize the root node
    this->root = new QuadNode(this, terrain, 0, dim-1, 0, dim-1);
}

void QuadTree::render(Vec2<float> camera_position, Vec2<float> camera_direction)
{
    this->camera_position = camera_position;
    this->camera_direction = normalize(camera_direction);
    
    // Bind the terrain texture
    glBindTexture(GL_TEXTURE_2D, this->texture_id);
    
    this->root->draw();
                
    // Unbind the vertex array object and texture
    glBindTexture(GL_TEXTURE_2D, 0);
}


bool QuadTree::isInFrustum(QuadNode *node)
{    
    Vec2<float> top_left_corner_direction = normalize(subtract(node->top_left_corner, this->camera_position));
    Vec2<float> top_right_corner_direction = normalize(subtract(node->top_right_corner, this->camera_position));
    Vec2<float> bottom_left_corner_direction = normalize(subtract(node->bottom_left_corner, this->camera_position));
    Vec2<float> bottom_right_corner_direction = normalize(subtract(node->bottom_right_corner, this->camera_position));
    
    if (dot(top_left_corner_direction, camera_direction) > fov_angle | dot(bottom_right_corner_direction, camera_direction) > fov_angle | dot(top_right_corner_direction, camera_direction) > fov_angle | dot(bottom_left_corner_direction, camera_direction) > fov_angle)
        return true;
    else
        return false;
}