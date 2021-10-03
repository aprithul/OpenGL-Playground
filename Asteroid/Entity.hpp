#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "MathUtil.hpp"
#include "Vertex.hpp"

#define MAX_VAOS 8

struct Transform
{
	Vec3f position = {};
	Vec3f rotation = {};
	Vec3f scale = {};

	Mat4x4 get_transform() const;
	Mat4x4 get_lookat(Vec3f target) const;
	Vec3f transform_point(Vec3f point);
	Vec3f transform_direction(Vec3f direction);
};

struct Entity
{
	Transform transform;
	float speed = 0;
	Vec3f velocity = {};
	Vec2f accumulated_force = {};
	float angular_speed = 0;
	int index_count;// s[MAX_VAOS] = {};
	//int vao_count = 0;
	unsigned int vao;// s[MAX_VAOS] = {};
	unsigned int texture_diffuse = 0;
	unsigned int texture_normal = 0;
	unsigned int texture_parallax = 0;
	unsigned int texture_specular = 0;
};


Entity make_space_ship();
Entity make_light_entity();
Entity make_quad();
Entity make_gameobject(const char* model_file_path, const char* diffuse_texture_path, const char* normal_texture_path, const char* parallex_texture_path, const char* specular_texture_path, bool flip, Float_32 import_scale);

Entity& make_camera();

extern Entity camera;



#endif // !ENTITY_HPP
