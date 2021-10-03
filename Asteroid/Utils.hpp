#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include "MathUtil.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

int read_file(const char*  file_path, char* file_content);
Vec3f calculate_tangent_in_tangent_space(Vec3f pos1, Vec3f pos2, Vec3f pos3, Vec2f uv1, Vec2f uv2, Vec2f uv3);
bool load_model(const std::string& pFile, std::vector<Float_32>& vertices, std::vector<Int_32 > &indices, Float_32 import_scale);
//void load_obj(const char* obj_name, std::vector<Float_32>& vertices, std::vector < std::vector<Int_32 >> &indices, Float_32 import_scale = 1.f);
Float_32 get_rand(float min, float max);
Vec3f lerp(const Vec3f& s, const Vec3f& e, float t);
Vec4f lerp(const Vec4f& s, const Vec4f& e, float t);
Float_32 clamp(Float_32 val, Float_32 min, Float_32 max);
#endif // !UTILS_HPP
