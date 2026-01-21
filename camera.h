#pragma once

#include "core.h"

struct Camera
{
    Vec3 position;
    Vec3 target;
    Vec3 up;
    float fov;

    Vec3 forward;
    Vec3 right;
    Vec3 realUp;
    float aspect;
    float angle;

    Camera(const Vec3 &pos, const Vec3 &lookAt, const Vec3 &upDir, float fovDeg, float aspectRatio)
        : position(pos), target(lookAt), up(upDir), fov(fovDeg), aspect(aspectRatio)
    {
        update();
    }

    void update()
    {
        forward = (target - position);
        forward.normalize();
        right = forward.cross(up);
        right.normalize();
        realUp = right.cross(forward);
        realUp.normalize();
        angle = std::tan(kPI * 0.5f * fov / 180.0f);
    }

    Ray makeRay(int x, int y, int width, int height) const
    {
        float invWidth = 1.0f / float(width);
        float invHeight = 1.0f / float(height);
        float px = (2.0f * ((x + 0.5f) * invWidth) - 1.0f) * angle * aspect;
        float py = (1.0f - 2.0f * ((y + 0.5f) * invHeight)) * angle;
        Vec3 dir = (forward + right * px + realUp * py);
        dir.normalize();
        return Ray(position, dir);
    }
};
