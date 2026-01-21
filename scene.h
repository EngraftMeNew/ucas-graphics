#pragma once

#include "objects.h"

#include <memory>
#include <vector>

struct Light
{
    Vec3 position;
    Vec3 emission;
    const Object *object = nullptr;
};

struct Scene
{
    std::vector<std::unique_ptr<Object>> objects;
    std::vector<Light> lights;

    bool intersect(const Ray &ray, HitInfo &hit) const
    {
        float tnear = std::numeric_limits<float>::infinity();
        bool hitSomething = false;
        HitInfo temp;
        for (const auto &obj : objects)
        {
            float t = 0.0f;
            if (obj->intersect(ray, t, temp))
            {
                if (t < tnear)
                {
                    tnear = t;
                    hit = temp;
                    hit.t = t;
                    hit.mat = &obj->material;
                    hitSomething = true;
                }
            }
        }
        return hitSomething;
    }

    bool occluded(const Ray &ray, float maxDist, const Object *ignore) const
    {
        HitInfo hit;
        for (const auto &obj : objects)
        {
            if (obj.get() == ignore)
                continue;
            float t = 0.0f;
            if (obj->intersect(ray, t, hit))
            {
                if (t > kEpsilon && t < maxDist)
                {
                    return true;
                }
            }
        }
        return false;
    }
};
