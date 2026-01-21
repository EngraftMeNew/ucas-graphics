#pragma once

#include "core.h"

class Object
{
public:
    Material material;
    virtual ~Object() = default;
    virtual bool intersect(const Ray &ray, float &t, HitInfo &hit) const = 0;
};

class Sphere : public Object
{
public:
    Vec3 center;
    float radius;
    float radius2;

    Sphere(const Vec3 &c, float r, const Material &m)
    {
        center = c;
        radius = r;
        radius2 = r * r;
        material = m;
    }

    bool intersect(const Ray &ray, float &t, HitInfo &hit) const override
    {
        Vec3 l = center - ray.origin;
        float tca = l.dot(ray.dir);
        if (tca < 0)
            return false;
        float d2 = l.dot(l) - tca * tca;
        if (d2 > radius2)
            return false;
        float thc = std::sqrt(radius2 - d2);
        float t0 = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0)
            t0 = t1;
        if (t0 < 0)
            return false;
        t = t0;
        Vec3 phit = ray.origin + ray.dir * t;
        Vec3 nhit = phit - center;
        nhit.normalize();
        hit.normal = nhit;
        return true;
    }
};

class Plane : public Object
{
public:
    Vec3 point;
    Vec3 normal;

    Plane(const Vec3 &p, const Vec3 &n, const Material &m)
    {
        point = p;
        normal = n;
        normal.normalize();
        material = m;
    }

    bool intersect(const Ray &ray, float &t, HitInfo &hit) const override
    {
        float denom = normal.dot(ray.dir);
        if (std::fabs(denom) < 1e-6f)
        {
            return false;
        }
        float t0 = (point - ray.origin).dot(normal) / denom;
        if (t0 < 0)
            return false;
        t = t0;
        hit.normal = normal;
        return true;
    }
};

class Box : public Object
{
public:
    Vec3 minCorner;
    Vec3 maxCorner;

    Box(const Vec3 &minC, const Vec3 &maxC, const Material &m)
    {
        minCorner = minC;
        maxCorner = maxC;
        material = m;
    }

    bool intersect(const Ray &ray, float &t, HitInfo &hit) const override
    {
        float tmin = -std::numeric_limits<float>::infinity();
        float tmax = std::numeric_limits<float>::infinity();
        Vec3 nmin(0.0f), nmax(0.0f);

        auto slab = [&](float origin, float dir, float minv, float maxv, const Vec3 &nNeg, const Vec3 &nPos) -> bool
        {
            if (std::fabs(dir) < 1e-8f)
            {
                return (origin >= minv && origin <= maxv);
            }
            float inv = 1.0f / dir;
            float t1 = (minv - origin) * inv;
            float t2 = (maxv - origin) * inv;
            Vec3 n1 = nNeg;
            Vec3 n2 = nPos;
            if (t1 > t2)
            {
                std::swap(t1, t2);
                std::swap(n1, n2);
            }
            if (t1 > tmin)
            {
                tmin = t1;
                nmin = n1;
            }
            if (t2 < tmax)
            {
                tmax = t2;
                nmax = n2;
            }
            return tmin <= tmax;
        };

        if (!slab(ray.origin.x, ray.dir.x, minCorner.x, maxCorner.x, Vec3(-1, 0, 0), Vec3(1, 0, 0)))
            return false;
        if (!slab(ray.origin.y, ray.dir.y, minCorner.y, maxCorner.y, Vec3(0, -1, 0), Vec3(0, 1, 0)))
            return false;
        if (!slab(ray.origin.z, ray.dir.z, minCorner.z, maxCorner.z, Vec3(0, 0, -1), Vec3(0, 0, 1)))
            return false;

        if (tmax < 0)
            return false;

        if (tmin >= 0)
        {
            t = tmin;
            hit.normal = nmin;
        }
        else
        {
            t = tmax;
            hit.normal = nmax;
        }
        return t >= 0;
    }
};
