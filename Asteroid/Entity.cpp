#include "Entity.hpp"
#include "Rendering.hpp"
#include "Utils.hpp"

#include <iostream>
#include <vector>



Mat4x4 Transform::get_lookat(Vec3f target) const
{
	Vec3f look_dir = (target - position).GetNormalized();
	Vec3f world_up = { 0,1,0 };
	Vec3f right = Cross(world_up, look_dir).GetNormalized();
	Vec3f up = Cross(look_dir, right).GetNormalized();
	Mat4x4 transform(right, up,look_dir);
	return transform;
}

Mat4x4 Transform::get_transform() const
{
	Mat3x3 rot_x = { 1, 0, 0,
					0, cosf(rotation.x), -sinf(rotation.x),
					0, sinf(rotation.x), cosf(rotation.x) };

	Mat3x3 rot_y = { cosf(rotation.y), 0, sinf(rotation.y),
				0, 1, 0,
				-sinf(rotation.y), 0,	cosf(rotation.y) };

	Mat3x3 rot_z = { cosf(rotation.z), -sinf(rotation.z), 0,
					sinf(rotation.z), cosf(rotation.z), 0,
					0,	0,	1 };

	Mat4x4 transform = rot_z * rot_y * rot_x;

	transform(0, 3) = position.x;
	transform(1, 3) = position.y;
	transform(2, 3) = position.z;

	return transform;
}

Vec3f Transform::transform_point(Vec3f point)
{
	Mat4x4 _transform = get_transform();
	return _transform * Vec4f{ point.x, point.y, point.z, 1.f };
}

Vec3f Transform::transform_direction(Vec3f direction)
{
	Mat4x4 _transform = get_transform();
	return _transform * Vec4f{ direction.x, direction.y, direction.z, 0.f };
}

Entity make_space_ship()
{
	Vertex vertices[3] = {
		{ 0.125f, -0.125f, 0, 1,1,1,0,0,1,0,0},
		{ 0, 0.125f, 0, 1,1,1,0,0,1,0,0},
		{ -0.125f, -0.125f, 0, 1,1,1,0,0,1,0,0},
	};
	int indices[3] = { 0,1,2 };
	Entity space_ship;
	space_ship.index_count = 3;
	space_ship.transform.position = { 0,0,1 };
	space_ship.transform.rotation = { 0,0,0 };
	space_ship.angular_speed = 1.0f;
	make_mesh(vertices, indices, 3, 3, space_ship.vao, 4, false);
	//space_ship.vao_count++;
	return space_ship;
}


Entity make_light_entity()
{
	Float_32 scale = 1;
	Float_32 r = 0.7, g = 0.7, b = 0;
	Vertex vertices[3] = {
		{ 0, 0, 0.3f* scale, r,g,b},
		{ -0.125f* scale, 0, 0, r,g,b},
		{ 0.125f * scale, 0, 0, r,g,b}
		
	};
	int indices[3] = { 0,1,2 };
	Entity light;
	light.index_count = 3;
	light.transform.position = { 0,2,0 };
	light.transform.rotation = { PI / 2, 0, 0 };// { PI / 2, 0, 0 };
	make_mesh(vertices, indices, 3, 3, light.vao, 2, false);
	//light.vao_count++;
	return light;
}

Entity make_quad()
{
	Vertex vertices[4] = {
	{ 1, 1, 0, 0,0,1,0,0,1,1,1},
	{ -1, 1, 0, 0,0,1,0,0,1,0,1},
	{ -1, -1, 0, 0,0,1,0,0,1,0,0},
	{ 1, -1, 0, 0,0,1,1,0,0,1,0}
	};
	int indices[6] = { 0,1,2,0,2,3 };
	Entity quad;
	quad.index_count = 6;
	quad.transform.position = { 0,0,0 };
	quad.transform.rotation = { 0,0,0 };
	quad.angular_speed = 1.0f;
	quad.texture_diffuse = 0;
	make_mesh(vertices, indices, 4, 6, quad.vao, 4, true);
	//quad.vao_count++;
	return quad;
}


Entity make_gameobject(const char* model_file_path, const char* diffuse_texture_path, const char* normal_texture_path, const char* parallex_texture_path, const char* specular_texture_path, bool flip, Float_32 import_scale)
{

	Entity entity;
	std::vector<Float_32> vertices;
	std::vector<Int_32> indices;
	//std::vector< std::vector<Int_32>> indices;
	//load_obj(obj_file_path, vertices, indices, import_scale);
	load_model(model_file_path, vertices, indices, import_scale);


	entity.index_count = indices.size();
	make_mesh(reinterpret_cast<Vertex*>(&vertices[0]), &indices[0], vertices.size() / 14, indices.size(), entity.vao, 5, false);
	//entity.vao_count++;

	/*int i = 0;
	for (auto& ind_arr : indices)
	{
		entity.index_counts[i] = ind_arr.size();
		make_mesh(reinterpret_cast<Vertex*>(&vertices[0]), &ind_arr[0], vertices.size() / 14, entity.index_counts[i], entity.vaos[i], 5, false);

		entity.vao_count++;
		i++;
	}*/
	entity.transform.position = { 0,0,0 };
	entity.transform.rotation = { 0,0,0 };

	// load texture
	make_texture(diffuse_texture_path, &entity.texture_diffuse, GL_SRGB_ALPHA, flip);
	make_texture(normal_texture_path, &entity.texture_normal, GL_RGB, flip);
	make_texture(parallex_texture_path, &entity.texture_parallax, GL_RGB, flip);
	make_texture(specular_texture_path, &entity.texture_specular, GL_RGB, flip);

	return entity;
}


Entity camera;
Entity& make_camera()
{
	//Entity camera;
	camera.transform.position = { 0,2,-2 };
	camera.transform.rotation = { PI / 4.f,0,0 };
	return camera;
}