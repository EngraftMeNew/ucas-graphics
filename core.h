#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

constexpr float kPI = 3.14159265358979323846f;
constexpr float kEpsilon = 1e-4f;
constexpr int kMaxDepth = 5;

struct Vec3
{
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float v) : x(v), y(v), z(v) {}
    Vec3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

    Vec3 operator+(const Vec3 &v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3 &v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    Vec3 operator*(float f) const { return Vec3(x * f, y * f, z * f); }
    Vec3 operator*(const Vec3 &v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 &operator+=(const Vec3 &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    Vec3 &operator*=(const Vec3 &v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    float dot(const Vec3 &v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3 &v) const
    {
        return Vec3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x);
    }
    float length2() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(length2()); }
    Vec3 &normalize()
    {
        float len = length();
        if (len > 0)
        {
            float inv = 1.0f / len;
            x *= inv;
            y *= inv;
            z *= inv;
        }
        return *this;
    }
};

inline float clamp01(float v)
{
    return std::max(0.0f, std::min(1.0f, v));
}

inline Vec3 clamp01(const Vec3 &v)
{
    return Vec3(clamp01(v.x), clamp01(v.y), clamp01(v.z));
}

inline float mix(float a, float b, float mixv)
{
    return b * mixv + a * (1.0f - mixv);
}

struct Ray
{
    Vec3 origin;
    Vec3 dir;
    Ray(const Vec3 &o, const Vec3 &d) : origin(o), dir(d) {}
};

struct Material
{
    Vec3 color = Vec3(1.0f);
    float reflection = 0.0f;
    float transparency = 0.0f;
    float ior = 1.1f;
    Vec3 emission = Vec3(0.0f);
};

struct HitInfo
{
    float t = 0.0f;
    Vec3 normal;
    const Material *mat = nullptr;
};
