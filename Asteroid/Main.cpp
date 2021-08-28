#include <stdio.h>
#include <iostream>
#include <chrono>
#include <functional>
#include <fstream>
#include <sstream>

#include "SDL.h"
#include <assert.h>
#include "Vertex.hpp"
#include "Entity.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MATH_UTIL_IMPLEMENTATION
#include "MathUtil.hpp"

#define GLEW_STATIC
#include "glew.h"

#define S_WIDTH 1200
#define S_HEIGHT 720

struct WorldData
{
	Vec3f light_dir;
	Vec3f camera_pos;
};

struct UniformLoc
{
	GLint mvp = 0;
	GLint model = 0;
	GLint texture_diffuse = 0;
	GLint texture_normal = 0;
	GLint texture_parallax = 0;
	GLint light_dir = 0;
	GLint camera_pos = 0;
	GLint lighting_mode = 0;
	
};

struct Shader
{
	GLint shader_program;
	UniformLoc uniform_loc;
	void load_uniform_locations()
	{
		uniform_loc.mvp = glGetUniformLocation(shader_program, "mvp");
		uniform_loc.model = glGetUniformLocation(shader_program, "model");
		uniform_loc.texture_diffuse = glGetUniformLocation(shader_program, "tex2d");
		uniform_loc.texture_normal = glGetUniformLocation(shader_program, "texNormal");
		uniform_loc.texture_parallax = glGetUniformLocation(shader_program, "parallaxMap");
		uniform_loc.light_dir = glGetUniformLocation(shader_program, "dir_light");
		uniform_loc.camera_pos = glGetUniformLocation(shader_program, "camera_pos");
		uniform_loc.lighting_mode = glGetUniformLocation(shader_program, "lighting_mode");
	}
};


SDL_Window* window;
SDL_GLContext gl_context;

int InitOpenglWindow()
{
	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		printf("SDL Video initialized successfully\n");
	}
	else
	{
		printf("SDL Video initialization failed\n");
		return 0;
	}


	// set opengl attributes before creating window
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1);

	// create window and opengl context
	window = SDL_CreateWindow("Asteroid", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, S_WIDTH, S_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	
	gl_context = SDL_GL_CreateContext(window);
	if (SDL_GL_GetCurrentContext() == NULL)
	{
		printf("OpenGL Context creation failed\n");
		return 0;
	}

	// load opengl functions using glew
	if (glewInit() == GLEW_OK)
	{
		printf("GLEW initialized successfully\n");
	}
	else
	{
		printf("GLEW initialization failed\n");
		return 0;
	}

	//set some opengl attributes
	glEnable(GL_BLEND);
	glBlendFunc(1, 0);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("Shading Language Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//printf("Extensions: %s\n", glGetString(GL_EXTENSIONS));

	return 1;
}

Vec3f calculate_tangent_in_tangent_space(Vec3f pos1, Vec3f pos2, Vec3f pos3, Vec2f uv1, Vec2f uv2, Vec2f uv3)
{
	Vec3f tangent = {};
	Float_32 d_u1 = uv2.x - uv1.x;
	Float_32 d_u2 = uv3.x - uv1.x;

	Float_32 d_v1 = uv2.y - uv1.y;
	Float_32 d_v2 = uv3.y - uv1.y;

	Float_32 det = (d_u1*d_v2 - d_u2 * d_v1);
	if (abs(det) <= EPS)	// determinant almost 0, can't calculate inverse
		return tangent;

	Float_32 det_inv = 1.f / det;
	
	Vec3f edge_1 = pos2 - pos1;
	Vec3f edge_2 = pos3 - pos1;

	tangent.x = det_inv * (d_v2 * edge_1.x - d_v1 * edge_2.x);
	tangent.y = det_inv * (d_v2 * edge_1.y - d_v1 * edge_2.y);
	tangent.z = det_inv * (d_v2 * edge_1.z - d_v1 * edge_2.z);
	return tangent.GetNormalized();

}

void read_file(const char*  file_path, char* file_content)
{
	std::ifstream _file;
	_file.open(file_path, std::fstream::in);
	std::stringstream _file_stream;
	_file_stream << _file.rdbuf();

	const std::string& str = _file_stream.str();
	const char* c_str = str.c_str();
	std::memcpy(file_content, c_str, str.size() + 1);

}

GLuint LoadShaders(const char* vertex_path, const char* fragment_path)
{

	char vertex_shader[4096];
	char fragment_shader[4096];

	read_file(vertex_path, vertex_shader);
	read_file(fragment_path, fragment_shader);

	GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);

	char* vs_p = &vertex_shader[0];
	glShaderSource(v_shader, 1, &vs_p, NULL);
	glCompileShader(v_shader);

	GLint v_compile_result;
	glGetShaderiv(v_shader, GL_COMPILE_STATUS, &v_compile_result);
	char log_data[1024];
	GLsizei log_len = 0;
	if (v_compile_result == GL_FALSE)
	{
		glGetShaderInfoLog(v_shader, 1024, &log_len, log_data);
		printf("%s\n", log_data);
	}
	else
		printf("Vertex Shader compilation successful\n");

	GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
	char* fs_p = &fragment_shader[0];
	glShaderSource(f_shader, 1, &fs_p, NULL);
	glCompileShader(f_shader);

	GLint f_compile_result;
	glGetShaderiv(f_shader, GL_COMPILE_STATUS, &f_compile_result);
	if (f_compile_result == GL_FALSE)
	{
		glGetShaderInfoLog(f_shader, 1024, &log_len, log_data);
		printf("%s\n", log_data);
	}
	else
		printf("Fragment Shader compilation successful\n");

	if (v_compile_result && f_compile_result)
	{
		GLuint shader_program = glCreateProgram();
		glAttachShader(shader_program, v_shader);
		glAttachShader(shader_program, f_shader);
		glLinkProgram(shader_program);

		GLint program_status;
		glGetProgramiv(shader_program, GL_LINK_STATUS, &program_status);
		
		if (program_status == GL_FALSE)
		{
			glGetProgramInfoLog(shader_program, 1024, &log_len, log_data);
			printf("%s\n", log_data);
		}
		else
		{
			printf("Shader program linking successful\n");
			return shader_program;
		}
	}

	return 0;

}

GLuint make_frame_buffer()
{
	GLuint fbo_color_attachment0;
	glGenTextures(1, &fbo_color_attachment0);
	glBindTexture(GL_TEXTURE_2D, fbo_color_attachment0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 640, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);


	GLuint fbo_color_attachment1;
	glGenTextures(1, &fbo_color_attachment1);
	glBindTexture(GL_TEXTURE_2D, fbo_color_attachment1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 640, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);


	GLuint fbo_depth_attachment;
	glGenTextures(1, &fbo_depth_attachment);
	glBindTexture(GL_TEXTURE_2D, fbo_depth_attachment);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 640, 640, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, (void*)0);

	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_color_attachment0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_color_attachment1, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo_depth_attachment, 0);
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
		printf("Frame buffer successfully created\n");
	else
		printf("frame buffer creation failed");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fbo;
}

void make_mesh(Vertex* vertices, Int_32* indices, Int_32 vertex_count, Int_32 index_count, Uint_32& vao, int attrib_count, bool is_textured)
{

	// make vbo and upload vertex data
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, vertices, GL_STATIC_DRAW);
	// make index buffer and upload index data
	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index_count, indices, GL_STATIC_DRAW);


	// make vao and set vertex attributes
	//GLuint vertex_array;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

	glEnableVertexAttribArray(0);
	//location in shader, size of attribute, type of attribute's component, sizeof vertex, first attribute offset (as void*)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*3));

	if (attrib_count >= 3)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
	}

	if(attrib_count >= 4)
	{
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9));
	}

	if (attrib_count >= 5)
	{
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 11));
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	// draw
	//glDrawElements(GL_LINE_LOOP, entity.index_count, GL_UNSIGNED_INT, (void*)0);

	/*GLenum err = glGetError();
	printf("%d\n", err);*/

}

void benchmark(int iterations)
{
	Mat4x4 m1 = Mat4x4::Identity();
	//Mat4x4 m2 = Mat4x4::Orthogrpahic(-1, 1, 5, 2, 6, 7);
	Float_32 aspect_ratio = 0;
	Mat4x4 m2 = Mat4x4::Perspective(90.f, 0.001, 100.f, aspect_ratio);

	auto start = std::chrono::high_resolution_clock::now();

	while (iterations-- > 0)
	{
		Mat4x4 m3 = m1 * m2;
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> _duration = (end - start);
	//std::cout <<_duration.count() << '\n';
	printf("Benchmark duration: %0.7lf\n", _duration.count());

}

int lighting_mode = 0;
void draw_entity(const Entity& entity, WorldData& world_data, const Mat4x4& projection, const Mat4x4& view, const Mat4x4& model, Shader& shader, GLenum mode=GL_LINE_LOOP)
{
	glUseProgram(shader.shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, entity.texture_diffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, entity.texture_normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, entity.texture_parallax);

	for (int i = 0; i < entity.vao_count; i++)
	{
		glBindVertexArray(entity.vaos[i]);
		glUniformMatrix4fv(shader.uniform_loc.model, 1, GL_FALSE, model.data);
		glUniformMatrix4fv(shader.uniform_loc.mvp, 1, GL_FALSE, (projection*view*model).data);
		glUniform1i(shader.uniform_loc.texture_diffuse, 0);
		glUniform1i(shader.uniform_loc.texture_normal, 1);
		glUniform1i(shader.uniform_loc.texture_parallax, 2);
		glUniform3f(shader.uniform_loc.light_dir, world_data.light_dir.x, world_data.light_dir.y, world_data.light_dir.z);
		glUniform3f(shader.uniform_loc.camera_pos, world_data.camera_pos.x, world_data.camera_pos.y, world_data.camera_pos.z);
		glUniform1i(shader.uniform_loc.lighting_mode, lighting_mode);

		glDrawElements(mode, entity.index_counts[i], GL_UNSIGNED_INT, (void*)0);
	}
}


void load_obj(const char* obj_name, std::vector<Float_32>& vertices, std::vector < std::vector<Int_32 >> &indices, Float_32 import_scale = 1.f)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_name,
		"Asset", true);

	if (ret)
		printf("Model loaded\n");
	else
		printf("Model loading failed\n");

	if (ret)
	{
		printf("Vertices %d\n", attrib.vertices.size() / 3);
		printf("Colors %d\n", attrib.colors.size() / 3);
		printf("Normals %d\n", attrib.normals.size() / 3);
		printf("Texcos %d\n", attrib.texcoords.size() / 2);
		printf("Shapes %d\n", shapes.size());
		printf("Materials %d\n", materials.size() );
	}
	if (!warn.empty())
		printf("Warning: %d\n", warn.c_str());
	else if (!err.empty())
		printf("Error: %d\n", err.c_str());


	// objs don't have concept of indices explicitely
	// easiest way to handle is to add every vertex in face to vertices array 
	// and add a new index for the vertex. This duplicates the vertex and doesn't take advantage of index buffer
	// but it's simple
	int indice = 0;
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		std::vector<int> shape_indices;

		

		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			//for calculating tangent
			Vec3f pos[3];
			Vec2f uv[3];

			// Loop over vertices in the face. first pass to calculate tangent
			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				pos[v].x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				pos[v].y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				pos[v].z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				uv[v].x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
				uv[v].y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
			}
			Vec3f tangent = calculate_tangent_in_tangent_space(pos[0], pos[1], pos[2], uv[0], uv[1], uv[2]);

			// 2nd pass
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				vertices.push_back(vx*import_scale);
				vertices.push_back(vy*import_scale);
				vertices.push_back(vz*import_scale);

				tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];
				vertices.push_back(red);
				vertices.push_back(green);
				vertices.push_back(blue);

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					vertices.push_back(nx);
					vertices.push_back(ny);
					vertices.push_back(nz);
				}
				else
				{
					vertices.push_back(0);
					vertices.push_back(0);
					vertices.push_back(0);
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					vertices.push_back(tx);
					vertices.push_back(ty);
					
				}
				else
				{
					vertices.push_back(0);
					vertices.push_back(0);
				}

				vertices.push_back(tangent.x);
				vertices.push_back(tangent.y);
				vertices.push_back(tangent.z);

				shape_indices.push_back(indice);
				indice++;
			}
			index_offset += fv;

			


			// per-face material
			shapes[s].mesh.material_ids[f];
		}
		indices.push_back(shape_indices);

	}

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
	Float_32 scale = 0.1f;
	Vertex vertices[3] = {
		{ 0.125f * scale, 0, 0.3f* scale, 1,1,1},
		{ -0.125f* scale, 0, 0.3f* scale, 1,1,1},
		{ 0, 0, 0, 0,0,1}
	};
	int indices[3] = { 0,1,2 };
	Entity light;
	light.index_counts[0] = 3;
	light.position = { 0,0.03,0 };
	light.rotation = {0, 0, 0};// { PI / 2, 0, 0 };
	make_mesh(vertices, indices, 3, 3, light.vaos[0], 2, false);
	light.vao_count++;
	return light;
}

Entity make_quad(GLuint texture)
{
	Vertex vertices[4] = {
	{ 1, 1, 0, 0,0,1,0,0,1,1,1},
	{ -1, 1, 0, 0,0,1,0,0,1,-1,1},
	{ -1, -1, 0, 0,0,1,0,0,1,-1,-1},
	{ 1, -1, 0, 0,0,1,1,0,0,1,-1}
	};
	int indices[6] = { 0,1,2,0,2,3};
	Entity quad;
	quad.index_counts[0] = 6;
	quad.position = { 0,0,0 };
	quad.rotation = { 0,0,0 };
	quad.angular_speed = 1.0f;
	quad.texture_diffuse = texture;
	make_mesh(vertices, indices, 4, 6, quad.vaos[0], 4, true);
	quad.vao_count++;
	return quad;
}


void make_texture(const char* texture_path, GLuint* texture, GLint internal_format)
{
	if (!texture_path)
		return;

	Int_32 width = 0;
	Int_32 height = 0;
	Int_32 comp = 0;
	stbi_uc* tex_data = stbi_load(texture_path, &width, &height, &comp, 0);
	if (!tex_data)
	{
		printf("Failed to load image. skipped texture creation\n");
		*texture = 0;
	}
	else
	{
		glGenTextures(1, texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *texture);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		assert(comp <= 4);

		GLenum format = GL_RGBA;
		if (comp == 3)
			format = GL_RGB;
		if (comp == 1)
			format = GL_R;

		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, tex_data);

	}
}

Entity make_gameobject(const char* obj_file_path, const char* diffuse_texture_path, const char* normal_texture_path, const char* parallex_texture_path, Float_32 import_scale)
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
	entity.position = { 0,0,0};
	entity.rotation = { 0,0,0 };

	// load texture
	make_texture(diffuse_texture_path, &entity.texture_diffuse, GL_RGBA);
	make_texture(normal_texture_path, &entity.texture_normal, GL_RGB);
	make_texture(parallex_texture_path, &entity.texture_parallax, GL_RGB);

	return entity;
}

Entity make_camera()
{
	Entity camera;
	camera.position = { 0,0.1,-0.1 };
	camera.rotation = { PI/4.f,0,0 };
	return camera;
}

Mat4x4 get_transform(Entity& ent)
{
	Mat3x3 rot_x = { 1, 0, 0,
					0, cosf(ent.rotation.x), -sinf(ent.rotation.x),
					0, sinf(ent.rotation.x), cosf(ent.rotation.x) };

	Mat3x3 rot_y = { cosf(ent.rotation.y), 0, sinf(ent.rotation.y),
				0, 1, 0,
				-sinf(ent.rotation.y), 0,	cosf(ent.rotation.y) };

	Mat3x3 rot_z = { cosf(ent.rotation.z), -sinf(ent.rotation.z), 0,
					sinf(ent.rotation.z), cosf(ent.rotation.z), 0,
					0,	0,	1}; 

	Mat4x4 transform = rot_z * rot_y * rot_x;

	transform(0, 3) = ent.position.x;
	transform(1, 3) = ent.position.y;
	transform(2, 3) = ent.position.z;

	return transform;
}


int main(int argc, char *args[])
{

	Vec3f pos1 = { 0,0,0 };
	Vec3f pos2 = { -1,-1,0 };
	Vec3f pos3 = { 1,1,0 };
	Vec2f uv1 = { 0.5,0 };
	Vec2f uv2 = { 0,1 };
	Vec2f uv3 = { 1,1 };

	Vec3f tangent = calculate_tangent_in_tangent_space(pos2, pos1, pos3, uv2, uv1, uv3);


	Float_32 aspect_ratio = 0;
	Mat4x4 projection = Mat4x4::Perspective(90, 0.001f, 10.f, aspect_ratio);
	static Vec2f cam_rot = {0,0};
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);


	//Mat4x4 projection = Mat4x4::Orthogrpahic(-2, 2, -2, 2, 10, -10);
	bool is_game_running = true;
	if (!InitOpenglWindow())
	{
		printf("Window creation with Opengl context failed\n");
		getchar();
		return 0;
	}

	Shader shader_textured;
	shader_textured.shader_program = LoadShaders("Shaders/diffuse.v", "Shaders/diffuse.f");
	shader_textured.load_uniform_locations();

	Shader shader_shape;
	shader_shape.shader_program = LoadShaders("Shaders/diffuse_line.v", "Shaders/diffuse_line.f");
	shader_shape.load_uniform_locations();


	if (!shader_textured.shader_program)
	{
		printf("Shader creation failed\n");
		getchar();
		return 0;
	}


	// make game world
	Entity camera = make_camera();
	//Entity sphere = make_gameobject("Asset/sphere.obj", "Asset/world.jpg", nullptr, 0.01f);
	//sphere.position = { -0.05f, 0, 0 };
	//Entity cube = make_gameobject("Asset/plane.obj", "Asset/brickwall.jpg", "Asset/brickwall_normal.jpg", 0.05f);
	//Entity cube = make_gameobject("Asset/cube.obj", "Asset/brickwall.jpg", "Asset/brickwall_normal.jpg", "Asset/brickwall_disp.jpg", 0.05f);
	Entity cube = make_gameobject("Asset/cube.obj", "Asset/bricks2.jpg", "Asset/bricks2_normal.jpg", "Asset/bricks2_disp.jpg", 0.05f);
	cube.position = { 0,-0.01,0 };
	cube.rotation = {0,-PI / 2,0 };
	Entity light = make_light_entity();

	SDL_Event game_event;
	// input variables
	int rotation_direction = 0;
	int forward_move_factor = 0;
	int right_move_factor = 0;
	int up_move_factor = 0;

	double last_time = SDL_GetTicks();
	float delta_time = 0;
	WorldData world_data;

	while (is_game_running)
	{
		
		Int_32 mouse_motion_x = 0;
		Int_32 mouse_motion_y = 0;
		// handle inputs
		while (SDL_PollEvent(&game_event))
		{
			switch (game_event.type)
			{
			
			case SDL_KEYDOWN:
			{
				switch (game_event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
					{
						is_game_running = false;
					}
					break;
					case SDLK_w:
					{
						up_move_factor = 1;
					}
					break;
					case SDLK_a:
					{
						right_move_factor = -1;
					}
					break;
					case SDLK_s:
					{
						up_move_factor = -1;
					}
					break;
					case SDLK_d:
					{
						right_move_factor = 1;
					}
					break;
					case SDLK_n:
					{
						lighting_mode = 1;
					}
					break;
					case SDLK_p:
					{
						lighting_mode = 2;
					}
					break;
					case SDLK_l:
					{
						lighting_mode = 0;
					}
					break;
				}

			}
				break;
			case SDL_KEYUP:
			{
				switch (game_event.key.keysym.sym)
				{
					case SDLK_w:
					case SDLK_s:
					{
						up_move_factor = 0;
					}
					break;
					case SDLK_a:
					case SDLK_d:
					{
						right_move_factor = 0;
					}
					break;
				}
			}
			break;
			case SDL_MOUSEMOTION:
			{
				mouse_motion_x = game_event.motion.xrel;
				mouse_motion_y = game_event.motion.yrel;
			}
			break;
			default:
				break;
			}
		}

		Float_32 light_rot_speed = 2.f;
		if (keystate[SDL_SCANCODE_LEFT])
			light.rotation.z -= light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_RIGHT])
			light.rotation.z += light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_UP])
			light.rotation.x -= light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_DOWN])
			light.rotation.x += light_rot_speed * delta_time;

		Mat4x4 light_transform = get_transform(light);

		world_data.light_dir = light_transform[2]; // forward vector == 3rd column
		world_data.light_dir.Normalize();

		// calculate entity transforms
		//Mat4x4 sphere_transform = get_transform(sphere);
		Mat4x4 cube_transform = get_transform(cube);

		// Set camera position and orientation
		Mat4x4 cam_transform = get_transform(camera);
		Vec3f cam_right = cam_transform[0];
		Vec3f cam_up = cam_transform[2];
		camera.position += (cam_right * right_move_factor + cam_up*up_move_factor)*delta_time*0.05f;
		Vec3f mouse_dir = { mouse_motion_y / (float)S_HEIGHT, mouse_motion_x / (float)S_WIDTH, 0 };
		camera.rotation += mouse_dir*delta_time*100.f;
		cam_transform = get_transform(camera);
		world_data.camera_pos = camera.position;


		// render
		glClearColor(0.3f, 0.3f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw_entity(sphere, world_data, projection, cam_transform.GetInverse(), sphere_transform, shader, GL_TRIANGLES);
		draw_entity(cube, world_data, projection, cam_transform.GetInverse(), cube_transform, shader_textured, GL_TRIANGLES);
		draw_entity(light, world_data, projection, cam_transform.GetInverse(), light_transform, shader_shape, GL_TRIANGLES);


		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		/*if (render_to_quad)
		{

		}
		else
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

			Int_32 sw = 0;
			Int_32 sh = 0;
			SDL_GetWindowSize(window, &sw, &sh);

			glBlitFramebuffer(0, 0, 640, 640, 0, 0, sw, sh, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}*/
		SDL_GL_SwapWindow(window);

		delta_time = (SDL_GetTicks() - last_time) / 1000.f;
		last_time = SDL_GetTicks();

	}

	return 0;
}