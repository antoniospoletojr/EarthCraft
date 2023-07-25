#include "QuadTree.h"

QuadTree::Node::Node(float min_x, float max_x, float min_z, float max_z)
    : min_x(min_x), max_x(max_x), min_z(min_z), max_z(max_z)
{
    children.fill(nullptr);
}

QuadTree::Node::~Node()
{
    for (Node *child : children)
    {
        delete child;
    }
}

QuadTree::QuadTree(float min_x, float max_x, float min_z, float max_z)
    : root(new Node(min_x, max_x, min_z, max_z)) {}

QuadTree::~QuadTree()
{
    delete root;
}

void QuadTree::build(const std::vector<GLuint> &indices)
{
    clear();
    for (GLuint index : indices)
    {
        this->root->indices.push_back(index);
        //Insert(index, root);
    }
}

void QuadTree::clear()
{
    for (Node *child : root->children)
    {
        delete child;
    }
    root->children.fill(nullptr);
    root->indices.clear();
}

void QuadTree::insert(GLuint index, Node *node)
{
    if (node->children[0] != nullptr)
    {
        for (Node *child : node->children)
        {
            if (isIndexInBounds(index, child))
            {
                insert(index, child);
                return;
            }
        }
    }

    node->indices.push_back(index);

    if (node->indices.size() > MaxIndicesPerNode && node->children[0] == nullptr)
    {
        subdivide(node);
    }
}

void QuadTree::subdivide(Node *node)
{
    float midX = (node->min_x + node->max_x) * 0.5f;
    float midY = (node->min_z + node->max_z) * 0.5f;

    node->children[0] = new Node(node->min_x, midX, node->min_z, midY);
    node->children[1] = new Node(midX, node->max_x, node->min_z, midY);
    node->children[2] = new Node(node->min_x, midX, midY, node->max_z);
    node->children[3] = new Node(midX, node->max_x, midY, node->max_z);

    for (GLuint index : node->indices)
    {
        for (Node *child : node->children)
        {
            if (isIndexInBounds(index, child))
            {
                insert(index, child);
                break;
            }
        }
    }

    node->indices.clear();
}

bool QuadTree::isIndexInBounds(GLuint index, Node *node) const
{
    // Implement the logic to determine if the index falls within the node's bounds
    // based on your specific requirements
    return true;
}

// La z varia come -sin(theta)
// La x varia come cos(theta)
std::vector<GLuint> *QuadTree::frustumCull(Vec3<float> position, Vec3<float> direction)
{
    // float dx1 = direction.x * cos((360 - CULLING_ANGLE) * (M_PI / 180.0)) - direction.z * sin((360 - CULLING_ANGLE) * (M_PI / 180.0));
    // float dz1 = direction.x * sin((360 - CULLING_ANGLE) * (M_PI / 180.0)) + direction.z * cos((360 - CULLING_ANGLE) * (M_PI / 180.0));
    
    // float dx2 = direction.x * cos(CULLING_ANGLE * (M_PI / 180.0)) - direction.z * sin(CULLING_ANGLE * (M_PI / 180.0));
    // float dz2 = direction.x * sin(CULLING_ANGLE * (M_PI / 180.0)) + direction.z * cos(CULLING_ANGLE * (M_PI / 180.0));
    
    // float m1 = dz1 / dx1;
    // float m2 = dz2 / dx2;
    
    // float q1 = position.z - m1 * position.x;
    // float q2 = position.z - m2 * position.x;

    // printf("position: %f, %f, %f\n", position.x, position.y, position.z);
    // printf("direction: %f, %f, %f\n", direction.x, direction.y, direction.z);
    // printf("dx1: %f, dz1: %f\n", dx1, dz1);
    // printf("dx2: %f, dz2: %f\n", dx2, dz2);
    // printf("m1: %f, m2: %f\n", m1, m2);
    // printf("q1: %f, q2: %f\n", q1, q2);
    // printf("\n");
    
    this->visible_indices.clear();
    float normalized_x, normalized_z;
    //printf("min_x: %f, max_x: %f, min_z: %f, max_z: %f\n", this->root->min_x, this->root->max_x, this->root->min_z, this->root->max_z);
    for (int j = this->root->min_z; j < this->root->max_z; j++) // 449
    {
        normalized_z = (float) j / this->root->max_z;
        for (int i = this->root->min_x; i < this->root->max_x; i++) // 902
        {
            normalized_x = (float) i / this->root->max_x;
            
            // // CHECK IF THE POINT IS IN THE FRUSTUM
            // float line1_z = m1 * normalized_x - q1;
            // float line2_z = m2 * normalized_x - q2;
            
            // if (((normalized_z > line1_z) && (normalized_z < line2_z))||((normalized_z < line1_z) && (normalized_z > line2_z)))
            // {
            //     this->visible_indices.push_back(this->root->indices[i + j * (this->root->max_x)]);
            // }
            this->visible_indices.push_back(this->root->indices[i + j * (this->root->max_x)]);
        }
        this->visible_indices.push_back(0xFFFFFFFFu);
    }

    return &this->visible_indices;
}

// void QuadTree::FrustumCull(const Node *node, std::vector<GLuint> &visibleIndices,
//                            /* frustum parameters */) const
// {
//     if (/* perform frustum culling on the node's bounds */)
//     {
//         if (node->children[0] != nullptr)
//         {
//             for (const Node *child : node->children)
//             {
//                 FrustumCull(child, visibleIndices, /* frustum parameters */);
//             }
//         }
//         else
//         {
//             visibleIndices.insert(visibleIndices.end(), node->indices.begin(), node->indices.end());
//         }
//     }
// }
