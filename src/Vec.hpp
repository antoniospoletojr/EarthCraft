/**
@file
@brief Vec structs.
*/

#ifndef VEC_HPP
#define VEC_HPP

#include <cmath>

/**
 * @brief 3D Vector struct.
 * 
 * @tparam T 
 */
template <typename T>
struct Vec3
{
    T x;
    T y;
    T z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(T x_val, T y_val, T z_val) : x(x_val), y(y_val), z(z_val) {}
};

/**
 * @brief 2D Vector struct.
 * 
 * @tparam T 
 */
template <typename T>
struct Vec2
{
    T u;
    T v;

    Vec2() : u(0), v(0) {}
    Vec2(T u_val, T v_val) : u(u_val), v(v_val) {}
};


/**
 * @brief Addition function to add two 3D vectors.
 * 
 * @tparam T 
 * @param v1 Vector 1
 * @param v2 Vector 2
 * @return Vec3<T> 
 */
template <typename T>
Vec3<T> add(const Vec3<T> &v1, const Vec3<T> &v2)
{
    Vec3<T> result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

/**
 * @brief Addition function to add two 2D vectors.
 * 
 * @tparam T 
 * @param v1 Vector 1
 * @param v2 Vector 2
 * @return Vec2<T> 
 */
template <typename T>
Vec2<T> add(const Vec2<T> &v1, const Vec2<T> &v2)
{
    Vec2<T> result;
    result.u = v1.u + v2.u;
    result.v = v1.v + v2.v;
    return result;
}

/**
 * @brief Subtraction function to subtract two 3D vectors.
 * 
 * @tparam T 
 * @param v1 Vector 1
 * @param v2 Vector 2
 * @return Vec3<T> 
 */
template <typename T>
Vec3<T> subtract(const Vec3<T> &v1, const Vec3<T> &v2)
{
    Vec3<T> result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

/**
 * @brief Subtraction function to subtract two 2D vectors.
 * 
 * @tparam T 
 * @param v1 Vector 1
 * @param v2 Vector 2
 * @return Vec2<T> 
 */
template <typename T>
Vec2<T> subtract(const Vec2<T> &v1, const Vec2<T> &v2)
{
    Vec2<T> result;
    result.u = v1.u - v2.u;
    result.v = v1.v - v2.v;
    return result;
}

/**
 * @brief Multiplication function to calculate the cross product of two 3D vectors.
 * 
 * @tparam T 
 * @param v1 Vector 1
 * @param v2 Vector 2
 * @return Vec3<T> 
 */
template <typename T>
Vec3<T> crossProduct(const Vec3<T> &v1, const Vec3<T> &v2)
{
    Vec3<T> result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

/**
 * @brief Normalization function to normalize a 3D vector.
 * 
 * @tparam T 
 * @param vector Vector to normalize
 * @return Vec3<T> 
 */
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

/**
 * @brief Normalization function to normalize a 2D vector.
 * 
 * @tparam T 
 * @param vector Vector to normalize
 * @return Vec2<T> 
 */
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

/**
 * @brief Dot product function to calculate the dot product of two 3D vectors.
 * 
 * @tparam T 
 * @param v1 Vector 1
 * @param v2 Vector 2
 * @return T 
 */
template <typename T>
T dot(const Vec3<T> &v1, const Vec3<T> &v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

/**
 * @brief Dot product function to calculate the dot product of two 2D vectors.
 * 
 * @tparam T 
 * @param v1 Vector 1
 * @param v2 Vector 2
 * @return T 
 */
template <typename T>
T dot(const Vec2<T> &v1, const Vec2<T> &v2)
{
    return v1.u * v2.u + v1.v * v2.v;
}

/**
 * @brief Calculate normal vector of a triangle using Newell's method.
 * 
 * @tparam T 
 * @param v1 First vertex
 * @param v2 Second vertex
 * @param v3 Third vertex
 * @return Vec3<T> 
 */
template <typename T>
Vec3<T> newellMethod(const Vec3<T> &v1, const Vec3<T> &v2, const Vec3<T> &v3)
{
    Vec3<T> result;
    result.x = (v1.y - v2.y) * (v1.z + v2.z) +
               (v2.y - v3.y) * (v2.z + v3.z) +
               (v3.y - v1.y) * (v3.z + v1.z);
    result.y = (v1.z - v2.z) * (v1.x + v2.x) +
               (v2.z - v3.z) * (v2.x + v3.x) +
               (v3.z - v1.z) * (v3.x + v1.x);
    result.z = (v1.x - v2.x) * (v1.y + v2.y) +
               (v2.x - v3.x) * (v2.y + v3.y) +
               (v3.x - v1.x) * (v3.y + v1.y);
    return result;
}


#endif