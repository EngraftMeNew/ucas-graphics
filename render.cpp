#include "render.h"

static Vec3 trace(const Ray &ray, const Scene &scene, int depth)
{
    HitInfo hit;
    if (!scene.intersect(ray, hit))
    {
        return Vec3(0.0f);
    }

    const Material &mat = *hit.mat;
    Vec3 phit = ray.origin + ray.dir * hit.t;
    Vec3 nhit = hit.normal;

    bool inside = false;
    if (ray.dir.dot(nhit) > 0)
    {
        nhit = -nhit;
        inside = true;
    }

    Vec3 surfaceColor(0.0f);
    Vec3 baseColor = mat.color;
    Vec3 ambient = baseColor * 0.08f;

    auto computeDirect = [&]() -> Vec3
    {
        Vec3 direct(0.0f);
        for (const auto &light : scene.lights)
        {
            Vec3 lightDir = light.position - phit;
            float lightDist = lightDir.length();
            lightDir.normalize();

            Ray shadowRay(phit + nhit * kEpsilon, lightDir);
            if (scene.occluded(shadowRay, lightDist, light.object))
            {
                continue;
            }
            float lambert = std::max(0.0f, nhit.dot(lightDir));
            direct += baseColor * lambert * light.emission;
        }
        return direct;
    };

    if ((mat.transparency > 0.0f || mat.reflection > 0.0f) && depth < kMaxDepth)
    {
        float facingratio = -ray.dir.dot(nhit);
        float fresnel = mix(std::pow(1.0f - facingratio, 3.0f), 1.0f, 0.1f);

        Vec3 reflectDir = ray.dir - nhit * 2.0f * ray.dir.dot(nhit);
        reflectDir.normalize();
        Vec3 reflection = trace(Ray(phit + nhit * kEpsilon, reflectDir), scene, depth + 1);

        Vec3 refraction(0.0f);
        if (mat.transparency > 0.0f)
        {
            float ior = mat.ior;
            float eta = inside ? ior : 1.0f / ior;
            float cosi = -nhit.dot(ray.dir);
            float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
            if (k >= 0.0f)
            {
                Vec3 refrDir = ray.dir * eta + nhit * (eta * cosi - std::sqrt(k));
                refrDir.normalize();
                refraction = trace(Ray(phit - nhit * kEpsilon, refrDir), scene, depth + 1);
            }
        }

        surfaceColor = reflection * (fresnel * mat.reflection) +
                       refraction * ((1.0f - fresnel) * mat.transparency);
        float diffuseWeight = (1.0f - mat.reflection) * (1.0f - mat.transparency);
        surfaceColor += computeDirect() * diffuseWeight + ambient;
    }
    else
    {
        surfaceColor = computeDirect() + ambient;
    }

    return surfaceColor + mat.emission;
}

std::vector<Vec3> render(const Scene &scene, const Camera &camera, int width, int height)
{
    std::vector<Vec3> img(static_cast<size_t>(width) * height);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Ray ray = camera.makeRay(x, y, width, height);
            Vec3 color = trace(ray, scene, 0);
            img[static_cast<size_t>(y) * width + x] = color;
        }
    }
    return img;
}
