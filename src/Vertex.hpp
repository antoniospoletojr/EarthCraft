#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <cmath>

// 3D Vertex struct
template <typename T>
struct Vertex3d
{
    T x;
    T y;
    T z;

    Vertex3d() : x(0), y(0), z(0) {}
};

// 2D Vertex struct
template <typename T>
struct Vertex2d
{
    T x;
    T y;

    Vertex2d() : x(0), y(0) {}
};

// 1D Vertex struct
template <typename T>
struct Vertex
{
    T x;

    Vertex() : x(0) {}
};

// Addition function to add two vertices
template <typename T>
Vertex3d<T> add(const Vertex3d<T> &v1, const Vertex3d<T> &v2)
{
    Vertex3d<T> result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

template <typename T>
Vertex2d<T> add(const Vertex2d<T> &v1, const Vertex2d<T> &v2)
{
    Vertex2d<T> result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    return result;
}

template <typename T>
Vertex<T> add(const Vertex<T> &v1, const Vertex<T> &v2)
{
    Vertex<T> result;
    result.x = v1.x + v2.x;
    return result;
}

// Subtraction function to subtract two vertices
template <typename T>
Vertex3d<T> subtract(const Vertex3d<T> &v1, const Vertex3d<T> &v2)
{
    Vertex3d<T> result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

template <typename T>
Vertex2d<T> subtract(const Vertex2d<T> &v1, const Vertex2d<T> &v2)
{
    Vertex2d<T> result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    return result;
}

template <typename T>
Vertex<T> subtract(const Vertex<T> &v1, const Vertex<T> &v2)
{
    Vertex<T> result;
    result.x = v1.x - v2.x;
    return result;
}

// Function to perform cross product of two 3D vectors
template <typename T>
Vertex3d<T> crossProduct(const Vertex3d<T> &v1, const Vertex3d<T> &v2)
{
    Vertex3d<T> result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

template <typename T>
Vertex3d<T> normalize(const Vertex3d<T> &vector)
{
    T length = std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
    Vertex3d<T> normalizedVector;

    if (length != 0)
    {
        normalizedVector.x = vector.x / length;
        normalizedVector.y = vector.y / length;
        normalizedVector.z = vector.z / length;
    }
    else
    {
        normalizedVector.x = 0;
        normalizedVector.y = 0;
        normalizedVector.z = 0;
    }

    return normalizedVector;
}

#endif
