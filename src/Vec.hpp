#ifndef VEC_HPP
#define VEC_HPP

#include <cmath>

// 3D Vector struct
template <typename T>
struct Vec3
{
    T x;
    T y;
    T z;
    
    Vec3() : x(0), y(0), z(0) {}
};

// 2D Vector struct
template <typename T>
struct Vec2
{
    T x;
    T y;

    Vec2() : x(0), y(0) {}
};

// 1D Vector struct
template <typename T>
struct Vec
{
    T x;

    Vec() : x(0) {}
};

// Addition function to add two vectors
template <typename T>
Vec3<T> add(const Vec3<T> &v1, const Vec3<T> &v2)
{
    Vec3<T> result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

template <typename T>
Vec2<T> add(const Vec2<T> &v1, const Vec2<T> &v2)
{
    Vec2<T> result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    return result;
}

template <typename T>
Vec<T> add(const Vec<T> &v1, const Vec<T> &v2)
{
    Vec<T> result;
    result.x = v1.x + v2.x;
    return result;
}

// Subtraction function to subtract two vectors
template <typename T>
Vec3<T> subtract(const Vec3<T> &v1, const Vec3<T> &v2)
{
    Vec3<T> result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

template <typename T>
Vec2<T> subtract(const Vec2<T> &v1, const Vec2<T> &v2)
{
    Vec2<T> result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    return result;
}

template <typename T>
Vec<T> subtract(const Vec<T> &v1, const Vec<T> &v2)
{
    Vec<T> result;
    result.x = v1.x - v2.x;
    return result;
}

// Function to perform cross product of two 3D vectors
template <typename T>
Vec3<T> crossProduct(const Vec3<T> &v1, const Vec3<T> &v2)
{
    Vec3<T> result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

template <typename T>
Vec3<T> normalize(const Vec3<T> &vector)
{
    T length = std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
    Vec3<T> normalized_vector;

    if (length != 0)
    {
        normalized_vector.x = vector.x / length;
        normalized_vector.y = vector.y / length;
        normalized_vector.z = vector.z / length;
    }
    else
    {
        normalized_vector.x = 0;
        normalized_vector.y = 0;
        normalized_vector.z = 0;
    }

    return normalized_vector;
}

#endif
