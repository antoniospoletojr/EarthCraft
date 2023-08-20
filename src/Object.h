#ifndef OBJECT_H
#define OBJECT_H

#include <GL/glew.h>
#include <vector>

typedef struct Object {
    // Buffer object descriptors
    GLuint vao;
    GLuint vbo;
    GLuint tbo;
    GLuint ibo;
    GLuint cbo;
    GLuint nbo;
    GLuint texture[2];
    
    // Buffers
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> textures;
    std::vector<GLuint> indices;
} Object;

#endif // OBJECT_H