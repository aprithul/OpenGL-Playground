#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "MathUtil.hpp"
#include "Vertex.hpp"
#define MAX_VAOS 8

struct Entity
{
	Vec3f position = {};
	Vec3f rotation = {};
	float speed = 0;
	Vec3f velocity = {};
	Vec2f accumulated_force = {};
	float angular_speed = 0;
	int index_counts[MAX_VAOS] = {};
	int vao_count = 0;
	unsigned int vaos[MAX_VAOS] = {};
	unsigned int texture_diffuse = 0;
	unsigned int texture_normal = 0;
};


#endif // !ENTITY_HPP
