#ifndef VERTEX_HPP
#define VERTEX_HPP

// 3D Vertex struct
template <typename T>
struct Vertex3d
{
    T x;
    T y;
    T z;
};

// 2D Vertex struct
template <typename T>
struct Vertex2d
{
    T x;
    T y;
};

// 1D Vertex struct
template <typename T>
struct Vertex
{
    T x;
};

#endif
