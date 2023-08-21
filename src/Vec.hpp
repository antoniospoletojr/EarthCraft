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
    Vec3(T x_val, T y_val, T z_val) : x(x_val), y(y_val), z(z_val) {}
};

// 2D Vector struct
template <typename T>
struct Vec2
{
    T u;
    T v;

    Vec2() : u(0), v(0) {}
    Vec2(T u_val, T v_val) : u(u_val), v(v_val) {}
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
    result.u = v1.u + v2.u;
    result.v = v1.v + v2.v;
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
    result.u = v1.u - v2.u;
    result.v = v1.v - v2.v;
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

template <typename T>
Vec2<T> normalize(const Vec2<T> &vector)
{
    T length = std::sqrt(vector.u * vector.u + vector.v * vector.v);
    Vec2<T> normalized_vector;

    if (length != 0)
    {
        normalized_vector.u = vector.u / length;
        normalized_vector.v = vector.v / length;
    }
    else
    {
        normalized_vector.u = 0;
        normalized_vector.v = 0;
    }
    return normalized_vector;
}

template <typename T>
T dot(const Vec3<T> &v1, const Vec3<T> &v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
T dot(const Vec2<T> &v1, const Vec2<T> &v2)
{
    return v1.u * v2.u + v1.v * v2.v;
}


#endif