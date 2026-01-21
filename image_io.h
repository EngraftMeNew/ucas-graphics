#pragma once

#include "core.h"

#include <fstream>
#include <string>
#include <vector>

bool writePNG(const std::string &path, const std::vector<Vec3> &img, int width, int height);
void writeGIFHeader(std::ofstream &ofs, int width, int height);
void writeGIFFrame(std::ofstream &ofs, const std::vector<unsigned char> &rgba, int width, int height, int delayCs);
void writeGIFFooter(std::ofstream &ofs);
std::vector<unsigned char> toRGBA(const std::vector<Vec3> &img);
