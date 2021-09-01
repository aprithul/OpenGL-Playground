#include "Entity.hpp"
#include "Rendering.hpp"
#include "Utils.hpp"
#include <vector>
Entity make_space_ship()
{
	Vertex vertices[3] = {
		{ 0.125f, -0.125f, 0, 1,1,1,0,0,1,0,0},
		{ 0, 0.125f, 0, 1,1,1,0,0,1,0,0},
		{ -0.125f, -0.125f, 0, 1,1,1,0,0,1,0,0},
	};
	int indices[3] = { 0,1,2 };
	Entity space_ship;
	space_ship.index_counts[0] = 3;
	space_ship.position = { 0,0,1 };
	space_ship.rotation = { 0,0,0 };
	space_ship.angular_speed = 1.0f;
	make_mesh(vertices, indices, 3, 3, space_ship.vaos[0], 4, false);
	space_ship.vao_count++;
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
	light.index_counts[0] = 3;
	light.position = { 0,2,0 };
	light.rotation = { PI / 2, 0, 0 };// { PI / 2, 0, 0 };
	make_mesh(vertices, indices, 3, 3, light.vaos[0], 2, false);
	light.vao_count++;
	return light;
}

//Entity make_quad(GLuint texture)
//{
//	Vertex vertices[4] = {
//	{ 1, 1, 0, 0,0,1,0,0,1,1,1},
//	{ -1, 1, 0, 0,0,1,0,0,1,-1,1},
//	{ -1, -1, 0, 0,0,1,0,0,1,-1,-1},
//	{ 1, -1, 0, 0,0,1,1,0,0,1,-1}
//	};
//	int indices[6] = { 0,1,2,0,2,3 };
//	Entity quad;
//	quad.index_counts[0] = 6;
//	quad.position = { 0,0,0 };
//	quad.rotation = { 0,0,0 };
//	quad.angular_speed = 1.0f;
//	quad.texture_diffuse = texture;
//	make_mesh(vertices, indices, 4, 6, quad.vaos[0], 4, true);
//	quad.vao_count++;
//	return quad;
//}


Entity make_gameobject(const char* obj_file_path, const char* diffuse_texture_path, const char* normal_texture_path, const char* parallex_texture_path, const char* specular_texture_path, bool flip, Float_32 import_scale)
{

	Entity entity;
	std::vector<Float_32> vertices;
	std::vector< std::vector<Int_32>> indices;
	load_obj(obj_file_path, vertices, indices, import_scale);
	int i = 0;
	for (auto& ind_arr : indices)
	{
		entity.index_counts[i] = ind_arr.size();
		make_mesh(reinterpret_cast<Vertex*>(&vertices[0]), &ind_arr[0], vertices.size() / 14, entity.index_counts[i], entity.vaos[i], 5, false);

		entity.vao_count++;
		i++;
	}
	entity.position = { 0,0,0 };
	entity.rotation = { 0,0,0 };

	// load texture
	make_texture(diffuse_texture_path, &entity.texture_diffuse, GL_SRGB_ALPHA, flip);
	make_texture(normal_texture_path, &entity.texture_normal, GL_RGB, flip);
	make_texture(parallex_texture_path, &entity.texture_parallax, GL_RGB, flip);
	make_texture(specular_texture_path, &entity.texture_specular, GL_RGB, flip);

	return entity;
}

Entity make_camera()
{
	Entity camera;
	camera.position = { 0,2,-2 };
	camera.rotation = { PI / 4.f,0,0 };
	return camera;
}