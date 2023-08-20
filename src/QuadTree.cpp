#include "QuadTree.h"

QuadNode::QuadNode(Terrain* terrain, float x, float width, float z, float depth)
{
    this->x = x;
    this->width = width;
    this->z = z;
    this->depth = depth;
    
    // Calculate node diagonal length
    float diag = sqrt(pow(width - x, 2) + pow(depth - z, 2));
    // Print diag and x, width, z and depth in one line
    printf("diag: %f, x: %f, width: %f, z: %f, depth: %f\n", diag, x, width, z, depth);
    
    if(diag > 160.f)
    {
        // Fill the 4 children with subportion of the terrain using the x, width, z and depth
        this->NW_child = new QuadNode(terrain, x, (width + x) / 2, z, (depth + z) / 2);
        this->NE_child = new QuadNode(terrain, (width + x) / 2, width, z, (depth + z) / 2);
        this->SW_child = new QuadNode(terrain, x, (width + x) / 2, (depth + z) / 2, depth);
        this->SE_child = new QuadNode(terrain, (width + x) / 2, width, (depth + z) / 2, depth);
        printf("\n");
        
    }
    else // this node is a leaf
    {
        int dim = terrain->getDim();
        Vec3<float> *map = terrain->getHeightmap();

        // Generate vertices, textures and nomals values for the mesh
        for (int i = x; i < width; i++)
        {
            for (int j = z; j < depth; j++)
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
        
        int delta = width - x;
        
        for (int j = 0; j < delta - 1; j++)               // 449
        {
            // Start a new strip
            this->object.indices.push_back(j * delta);
            for (int i = 0; i < delta-1; i++)               // 902
            {
                // Add vertices to strip
                this->object.indices.push_back((j + 1) * delta + i);
                this->object.indices.push_back(j * delta + i);
            }
            // Use primitive restart to start a new strip
            this->object.indices.push_back(0xFFFFFFFFu);
        }

        
        
        // Calculate normals
        for (int i = 0; i < this->object.indices.size()-3; i += 2)
        {    
            if (this->object.indices[i+1] == 0xFFFFFFFFu)
                continue;
            
            // Get the indices of the triangle
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
            
            // Get the vertices of the triangle
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

        // Fill the 4 children with nullptr
        this->NW_child = nullptr;
        this->NE_child = nullptr;
        this->SW_child = nullptr;
        this->SE_child = nullptr;
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
    
    // Reset mesh arrays if they are not empty
    this->object.vertices.clear();
    this->object.indices.clear();
    this->object.textures.clear();
    this->object.normals.clear();

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
    
    // children.fill(nullptr);
}

QuadNode::~QuadNode()
{

}

void QuadNode::draw()
{
    if (this->NW_child == nullptr)
    {
        // Bind the terrain texture
        glBindTexture(GL_TEXTURE_2D, this->object.texture[0]);
        
        // Draw the terrain
        glBindVertexArray(this->object.vao);
        
        // Enable two vertex arrays: co-ordinates and color.
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        
        glEnable(GL_PRIMITIVE_RESTART);                                                                 // Enable primitive restart
        glDrawElements(GL_TRIANGLE_STRIP, this->object.indices.size(), GL_UNSIGNED_INT, 0);      // Draw the triangles
        glDisable(GL_PRIMITIVE_RESTART);
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        
        // Unbind the vertex array object and texture
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    // Draw the children
    if (this->NW_child != nullptr)
        this->NW_child->draw();
    if (this->NE_child != nullptr)
        this->NE_child->draw();
    if (this->SW_child != nullptr)
        this->SW_child->draw();
    if (this->SE_child != nullptr)
        this->SE_child->draw();
}

QuadTree::QuadTree()
{

}


QuadTree::~QuadTree()
{

}

void QuadTree::initialize(Terrain *terrain)
{
    printf("Building quadtree...\n");

    // Get the dimension of the map, useful for allocations
    int dim = terrain->getDim();
    
    // Initialize the root node
    this->root = new QuadNode(terrain, 0, dim, 0, dim);
}

void QuadTree::draw()
{
    // Draw the root node
    this->root->draw();
}