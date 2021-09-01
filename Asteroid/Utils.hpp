#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include "MathUtil.hpp"

void read_file(const char*  file_path, char* file_content);
Vec3f calculate_tangent_in_tangent_space(Vec3f pos1, Vec3f pos2, Vec3f pos3, Vec2f uv1, Vec2f uv2, Vec2f uv3);
void load_obj(const char* obj_name, std::vector<Float_32>& vertices, std::vector < std::vector<Int_32 >> &indices, Float_32 import_scale = 1.f);

#endif // !UTILS_HPP
