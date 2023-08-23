/**
@file
@brief Object struct.
*/

#ifndef OBJECT_H
#define OBJECT_H

#include <GL/glew.h>
#include <vector>

/**
 * @brief Contains the OpenGL object descriptors and the buffers.
 */
typedef struct Object {
    // Buffer object descriptors
    GLuint vao;             ///< Vertex array object
    GLuint vbo;             ///< Vertex buffer object
    GLuint tbo;             ///< Texture buffer object
    GLuint ibo;             ///< Index buffer object
    GLuint cbo;             ///< Color buffer object
    GLuint nbo;             ///< Normal buffer object
    GLuint texture[2];      ///< Texture ids
    
    // Buffers
    std::vector<GLfloat> vertices;  ///< Vertex buffer
    std::vector<GLfloat> colors;    ///< Color buffer
    std::vector<GLfloat> normals;   ///< Normal buffer
    std::vector<GLfloat> textures;  ///< Texture buffer
    std::vector<GLuint> indices;    ///< Index buffer
} Object;

#endif // OBJECT_H