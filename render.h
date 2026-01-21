#pragma once

#include "camera.h"
#include "scene.h"

#include <vector>

std::vector<Vec3> render(const Scene &scene, const Camera &camera, int width, int height);
