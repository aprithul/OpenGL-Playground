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
	GLint texture = 0;
	GLint light_dir = 0;
	GLint camera_pos = 0;
}uniform_loc;

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

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("Shading Language Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//printf("Extensions: %s\n", glGetString(GL_EXTENSIONS));

	return 1;
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

	char vertex_shader[1024];
	char fragment_shader[1024];

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

void MakeMesh(Vertex* vertices, Int_32* indices, Int_32 vertex_count, Int_32 index_count, Uint_32& vao, bool is_textured)
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

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));


	//if (is_textured)
	{
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9));
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


void draw_entity(const Entity& entity, WorldData world_data, const Mat4x4& projection, const Mat4x4& view, const Mat4x4& model, GLenum mode=GL_LINE_LOOP)
{
	glBindTexture(GL_TEXTURE0, entity.texture);
	for (int i = 0; i < entity.vao_count; i++)
	{
		glBindVertexArray(entity.vaos[i]);
		glUniformMatrix4fv(uniform_loc.model, 1, GL_FALSE, model.data);
		glUniformMatrix4fv(uniform_loc.mvp, 1, GL_FALSE, (projection*view*model).data);
		glUniform1i(uniform_loc.texture, GL_TEXTURE0);
		glUniform3f(uniform_loc.light_dir, world_data.light_dir.x, world_data.light_dir.y, world_data.light_dir.z);
		glUniform3f(uniform_loc.camera_pos, world_data.camera_pos.x, world_data.camera_pos.y, world_data.camera_pos.z);

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
	MakeMesh(vertices, indices, 3, 3, space_ship.vaos[0], false);
	space_ship.vao_count++;
	return space_ship;
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
	quad.texture = texture;
	MakeMesh(vertices, indices, 4, 6, quad.vaos[0], true);
	quad.vao_count++;
	return quad;
}

Entity make_cube(const char* obj_file_path)
{

	Entity cube;
	std::vector<Float_32> vertices;
	std::vector< std::vector<Int_32>> indices;
	load_obj(obj_file_path, vertices, indices, 0.01f);
	int i = 0;
	for (auto& ind_arr : indices)
	{
		cube.index_counts[i] = ind_arr.size();
		MakeMesh(reinterpret_cast<Vertex*>(&vertices[0]), &ind_arr[0], vertices.size() / 11, cube.index_counts[i], cube.vaos[i], false);

		cube.vao_count++;
		i++;
	}
	cube.position = { 0,0,0};
	cube.rotation = { 0,0,0 };

	// load texture

	Int_32 width = 0;
	Int_32 height = 0;
	Int_32 comp = 0;
	stbi_uc* tex_data = stbi_load("Asset/cube_diffuse.png", &width, &height, &comp, 0);
	if (!tex_data)
	{
		printf("Failed to load image. skipped texture creation\n");
		cube.texture = 0;
	}
	else
	{
		glGenTextures(1, &cube.texture);
		glBindTexture(GL_TEXTURE_2D, cube.texture);
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);

	}
	return cube;
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
	Float_32 aspect_ratio = 0;
	Mat4x4 projection = Mat4x4::Perspective(90, 0.001f, 100.f, aspect_ratio);
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

	GLint shader_program = LoadShaders("Shaders/diffuse.v", "Shaders/diffuse.f");

	if (!shader_program)
	{
		printf("Shader creation failed\n");
		getchar();
		return 0;
	}
	glUseProgram(shader_program);
	uniform_loc.mvp = glGetUniformLocation(shader_program, "mvp");
	uniform_loc.model = glGetUniformLocation(shader_program, "model");
	uniform_loc.texture = glGetUniformLocation(shader_program, "tex2d");
	uniform_loc.light_dir = glGetUniformLocation(shader_program, "dir_light");
	uniform_loc.camera_pos = glGetUniformLocation(shader_program, "camera_pos");
	// make game world
	//GLuint fbo = make_frame_buffer();
	//Entity space_ship = make_space_ship();
	//Entity quad = make_quad(0);
	Entity camera = make_camera();
	Entity cube = make_cube("Asset/sphere.obj");
	//Entity cube = make_cube("Asset/treasure_chest.obj");
	WorldData render_data;
	render_data.light_dir = {0,-1.f,1.f};
	render_data.light_dir.Normalize();
	SDL_Event game_event;
	int rotation_direction = 0;
	int forward_move_factor = 0;
	int right_move_factor = 0;
	int up_move_factor = 0;



	double last_time = SDL_GetTicks();
	float delta_time = 0;
	//benchmark(1000000);
	bool render_to_quad = 0;
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
					case SDLK_q:
					{
						render_to_quad = !render_to_quad;
					}
					default:
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
			render_data.light_dir.x -= light_rot_speed * delta_time;
		else if (keystate[SDL_SCANCODE_RIGHT])
			render_data.light_dir.x += light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_UP])
			render_data.light_dir.z += light_rot_speed * delta_time;
		else if (keystate[SDL_SCANCODE_DOWN])
			render_data.light_dir.z -= light_rot_speed *delta_time;

		render_data.light_dir.Normalize();
		printf("%f,%f,%f\n", render_data.light_dir.x, render_data.light_dir.y, render_data.light_dir.z);
		//space_ship.rotation.z += rotation_direction * delta_time * space_ship.angular_speed;

		// Space_ship stuff
		//Mat4x4 space_ship_transform = get_transform(space_ship);

		// Cube stuff
		Mat4x4 cube_transform = get_transform(cube);


		// Camera transform stuff
		Mat4x4 cam_transform = get_transform(camera);
		Vec3f cam_right = cam_transform[0];
		Vec3f cam_up = cam_transform[2];
		camera.position += (cam_right * right_move_factor + cam_up*up_move_factor)*delta_time;
		Vec3f mouse_dir = { mouse_motion_y / (float)S_HEIGHT, mouse_motion_x / (float)S_WIDTH, 0 };
		camera.rotation += mouse_dir*delta_time*100.f;
		
		//printf("%f, %f\n", camera.rotation.x, camera.rotation.y);
		cam_transform = get_transform(camera);
		render_data.camera_pos = camera.position;


		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glClearColor(0.3f, 0.3f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//draw_entity(quad, Mat4x4::Identity(), GL_TRIANGLES);
		//draw_entity(space_ship, projection * cam_transform.GetInverse() * space_ship_transform, GL_TRIANGLES);
		draw_entity(cube, render_data, projection, cam_transform.GetInverse(), cube_transform, GL_TRIANGLES);


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