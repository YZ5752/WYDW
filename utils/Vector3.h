#pragma once

#include <cmath>

class Vector3 {
public:
    double x, y, z;

    // 构造函数
    Vector3() : x(0), y(0), z(0) {}
    Vector3(double x, double y, double z) : x(x), y(y), z(z) {}

    // 向量加法
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    // 向量减法
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    // 向量点乘
    double dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // 向量叉乘
    Vector3 cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // 向量模长
    double magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // 向量模长的平方
    double magnitudeSquared() const {
        return x * x + y * y + z * z;
    }

    // 向量归一化
    Vector3 normalize() const {
        double mag = magnitude();
        if (mag == 0) return Vector3();
        return Vector3(x / mag, y / mag, z / mag);
    }

    // 标量乘法
    Vector3 operator*(double scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    // 标量除法
    Vector3 operator/(double scalar) const {
        if (scalar == 0) return Vector3();
        return Vector3(x / scalar, y / scalar, z / scalar);
    }
}; 