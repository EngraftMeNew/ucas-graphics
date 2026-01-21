#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "camera.h"
#include "image_io.h"
#include "objects.h"
#include "render.h"
#include "scene.h"

namespace fs = std::filesystem;

static Scene buildScene()
{
    Scene scene;

    Material lightMat;
    lightMat.emission = Vec3(4.0f, 4.0f, 4.0f);
    lightMat.color = Vec3(0.0f);

    Material groundMat;
    groundMat.color = Vec3(0.0f, 0.0f, 0.0f);

    Material redMat;
    redMat.color = Vec3(0.9f, 0.15f, 0.15f);
    redMat.reflection = 0.6f;
    redMat.transparency = 0.25f;
    redMat.ior = 1.3f;

    Material greenMat;
    greenMat.color = Vec3(0.15f, 0.9f, 0.2f);
    greenMat.reflection = 0.6f;
    greenMat.transparency = 0.25f;
    greenMat.ior = 1.3f;

    Material blueMat;
    blueMat.color = Vec3(0.2f, 0.35f, 0.95f);
    blueMat.reflection = 0.6f;
    blueMat.transparency = 0.25f;
    blueMat.ior = 1.3f;

    Material yellowMat;
    yellowMat.color = Vec3(0.95f, 0.9f, 0.15f);
    yellowMat.reflection = 0.2f;

    auto lightSphere = std::make_unique<Sphere>(Vec3(0.0f, 4.5f, -5.0f), 0.7f, lightMat);
    scene.lights.push_back({lightSphere->center, lightMat.emission, lightSphere.get()});
    scene.objects.push_back(std::move(lightSphere));

    scene.objects.push_back(std::make_unique<Plane>(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), groundMat));
    scene.objects.push_back(std::make_unique<Sphere>(Vec3(-1.8f, -0.1f, -6.0f), 1.1f, redMat));
    scene.objects.push_back(std::make_unique<Sphere>(Vec3(1.6f, -0.2f, -5.5f), 1.0f, greenMat));
    scene.objects.push_back(std::make_unique<Sphere>(Vec3(0.0f, -0.3f, -4.0f), 0.9f, blueMat));
    scene.objects.push_back(std::make_unique<Box>(Vec3(-0.7f, -1.0f, -8.2f), Vec3(0.7f, 0.4f, -6.8f), yellowMat));

    return scene;
}

int main(int argc, char **argv)
{
    bool quick = false;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--quick")
        {
            quick = true;
        }
    }

    int stillWidth = quick ? 320 : 1280;
    int stillHeight = quick ? 320 : 1280;
    int animWidth = quick ? 320 : 960;
    int animHeight = quick ? 320 : 960;
    int frames = quick ? 30 : 60;

    fs::path baseDir = fs::current_path();
    fs::path outputDir = baseDir / "output";
    fs::path stillDir = outputDir / "stills";
    fs::path frameDir = outputDir / "frames";
    fs::create_directories(stillDir);
    fs::create_directories(frameDir);

    Scene scene = buildScene();

    float camHeight = 1.5f;
    Vec3 target(0.0f, camHeight, -5.5f);
    Vec3 up(0.0f, 1.0f, 0.0f);
    float radius = 6.5f;

    float fov = 60.0f;
    std::vector<float> stillAngles = {0.0f, 90.0f, 180.0f, 270.0f};
    for (size_t i = 0; i < stillAngles.size(); ++i)
    {
        float rad = stillAngles[i] * kPI / 180.0f;
        Vec3 pos(target.x + std::cos(rad) * radius, camHeight, target.z + std::sin(rad) * radius);
        Camera cam(pos, target, up, fov, float(stillWidth) / float(stillHeight));
        auto img = render(scene, cam, stillWidth, stillHeight);
        fs::path file = stillDir / ("shot_" + std::to_string(i) + ".png");
        if (writePNG(file.string(), img, stillWidth, stillHeight))
        {
            std::cout << "Wrote " << file.string() << std::endl;
        }
    }

    fs::path gifPath = outputDir / "animation.gif";
    std::ofstream gif(gifPath, std::ios::binary);
    if (!gif)
    {
        std::cerr << "Failed to open GIF for writing." << std::endl;
        return 1;
    }
    writeGIFHeader(gif, animWidth, animHeight);

    for (int f = 0; f < frames; ++f)
    {
        float t = float(f) / float(frames);
        float angle = t * 2.0f * kPI;
        Vec3 pos(target.x + std::cos(angle) * radius, camHeight, target.z + std::sin(angle) * radius);
        Camera cam(pos, target, up, fov, float(animWidth) / float(animHeight));
        auto img = render(scene, cam, animWidth, animHeight);
        fs::path frameFile = frameDir / ("frame_" + std::to_string(10000 + f).substr(1) + ".png");
        writePNG(frameFile.string(), img, animWidth, animHeight);
        auto rgba = toRGBA(img);
        writeGIFFrame(gif, rgba, animWidth, animHeight, 4);
        std::cout << "Rendered frame " << f + 1 << "/" << frames << std::endl;
    }

    writeGIFFooter(gif);
    gif.close();
    std::cout << "Wrote " << gifPath.string() << std::endl;

    return 0;
}
