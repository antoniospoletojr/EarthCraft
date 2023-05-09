#ifndef VEC3_HPP
#define VEC3_HPP

template<typename T>
class Vec3 {
public:
    // Constructors
    Vec3() : x_(0), y_(0), z_(0) {}
    Vec3(T x, T y, T z) : x_(x), y_(y), z_(z) {}
    Vec3(const Vec3& other) : x_(other.x_), y_(other.y_), z_(other.z_) {}
    
    // Assignment operator
    Vec3& operator=(const Vec3& other)
    {
        if (this != &other) {
            x_ = other.x_;
            y_ = other.y_;
            z_ = other.z_;
        }
        return *this;
    }

    // Getters
    T x() const
    {
        return x_;
    }

    T y() const
    {
        return y_;
    }

    T z() const
    {
        return z_;
    }

    // Setters
    void set_x(T x)
    {
        x_ = x;
    }

    void set_y(T y)
    {
        y_ = y;
    }

    void set_z(T z)
    {
        z_ = z;
    }

    // Vector arithmetic operations
    Vec3 operator+(const Vec3& other) const
    {
        return Vec3(x_ + other.x_, y_ + other.y_, z_ + other.z_);
    }

    Vec3 operator-(const Vec3& other) const
    {
        return Vec3(x_ - other.x_, y_ - other.y_, z_ - other.z_);
    }

    Vec3 operator*(T scalar) const
    {
        return Vec3(x_ * scalar, y_ * scalar, z_ * scalar);
    }

    T dot(const Vec3& other) const
    {
        return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
    }

    Vec3 cross(const Vec3& other) const
    {
        return Vec3(y_ * other.z_ - z_ * other.y_, z_ * other.x_ - x_ * other.z_, x_ * other.y_ - y_ * other.x_);
    }

    // Vector comparison operations
    bool operator==(const Vec3& other) const
    {
        return x_ == other.x_ && y_ == other.y_ && z_ == other.z_;
    }

    bool operator!=(const Vec3& other) const
    {
        return !(*this == other);
    }

private:
    T x_;
    T y_;
    T z_;
};

#endif