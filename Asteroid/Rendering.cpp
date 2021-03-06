#define MATH_UTIL_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define GLEW_STATIC

#include <iostream>
#include "Rendering.hpp"
#include "Utils.hpp"


SDL_Window* window;
SDL_GLContext gl_context;



int InitOpenglWindow(Int_32 width, Int_32 height)
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
	window = SDL_CreateWindow("Asteroid", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height
, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);
	//glEnable(GL_CULL_FACE);

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("Shading Language Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//printf("Extensions: %s\n", glGetString(GL_EXTENSIONS));

	return 1;
}

Shader::Shader(const char* vertex_path, const char* fragment_path)
{
	char vertex_shader[8192];
	char fragment_shader[8192];

	assert(read_file(vertex_path, vertex_shader));
	assert(read_file(fragment_path, fragment_shader));

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
		assert(0);
	}
	else
		printf("Vertex Shader  %s : compilation successful\n", vertex_path);

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
		assert(0);
	}
	else
		printf("Fragment Shader %s : compilation successful\n", fragment_path);

	if (v_compile_result && f_compile_result)
	{
		shader_program = glCreateProgram();
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
			glDeleteShader(v_shader);
			glDeleteShader(f_shader);
			printf("Shader program linking successful\n");
		}
	}
}

Shader::~Shader()
{
	glDeleteProgram(shader_program);
}

GLuint make_depth_framebuffer(int w, int h, GLuint* depth_texture)
{
	GLuint fbo = 0;
	glGenFramebuffers(1, &fbo);

	//GLuint fbo_depth_attachment;
	glGenTextures(1, depth_texture);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glBindTexture(GL_TEXTURE_2D, *depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
		printf("Frame buffer successfully created\n");
	else
		printf("frame buffer creation failed");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fbo;
}

GLuint make_color_framebuffer(int w, int h, GLuint* color_texture, bool has_depth_buffer)
{
	
	glGenTextures(1, color_texture);
	glBindTexture(GL_TEXTURE_2D, *color_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);

	GLuint fbo_depth_attachment;
	glGenTextures(1, &fbo_depth_attachment);
	glBindTexture(GL_TEXTURE_2D, fbo_depth_attachment);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, (void*)0);


	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *color_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo_depth_attachment, 0);
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status == GL_FRAMEBUFFER_COMPLETE)
		printf("Frame buffer successfully created\n");
	else
		printf("frame buffer creation failed");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fbo;
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));

	if (attrib_count >= 3)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
	}

	if (attrib_count >= 4)
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

void make_particle_mesh(ParticleEmitter& particle_emmiter)
{

	static VertexMin vertices[4] = {
		{0.01f, 0.01f, 0, 0, 0},
		{-0.01f, 0.01f, 0, 1, 0},
		{-0.01f, -0.01f, 0, 1, 1},
		{0.01f, -0.01f, 0, 0, 1},
	};
	static int indices[6] = { 0,1,2,0,2,3 };

	// vbo to store vertex position data
	GLuint vertex_position_vbo;
	glGenBuffers(1, &vertex_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexMin)*4, vertices, GL_STATIC_DRAW);
	
	// make index buffer and upload index data
	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6, indices, GL_STATIC_DRAW);

	// make vbo for instancing ( contains color and position offsets)
	int stride = (sizeof(Vec4f) * 4) + sizeof(Vec4f);
	glGenBuffers(1, &particle_emmiter.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, particle_emmiter.vbo);
	glBufferData(GL_ARRAY_BUFFER, stride*ParticleEmitter::max_particle_count, nullptr, GL_STREAM_DRAW);


	// make vao and set vertex attributes
	//GLuint vertex_array;
	glGenVertexArrays(1, &particle_emmiter.vaos[0]);
	glBindVertexArray(particle_emmiter.vaos[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_position_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexMin), (void*)(sizeof(float) * 0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexMin), (void*)(sizeof(float) * 3));

	glBindBuffer(GL_ARRAY_BUFFER, particle_emmiter.vbo);
	
	
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(Vec4f) * 0));
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(Vec4f) * 1));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(Vec4f) * 2));
	glVertexAttribDivisor(4, 1);
	
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(Vec4f) * 3));
	glVertexAttribDivisor(5, 1);

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(Vec4f) * 4));
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


struct InstanceData
{
	Mat4x4 instance_transform;
	Vec4f color;
};

int compare_Particles(const void* a, const void* b)
{
	InstanceData arg1 = *(const InstanceData*)a;
	InstanceData arg2 = *(const InstanceData*)b;

	if (arg1.instance_transform(2,3) < arg2.instance_transform(2, 3)) return 1;
	if (arg1.instance_transform(2, 3) > arg2.instance_transform(2, 3)) return -1;
	return 0;

	// return (arg1 > arg2) - (arg1 < arg2); // possible shortcut
	// return arg1 - arg2; // erroneous shortcut (fails if INT_MIN is present)
}

void draw_particle_emitter(ParticleEmitter& particle_emitter, Shader& shader, const Mat4x4& view, const Mat4x4& projection, GLenum mode)
{
	// set the position data in vbo
	static InstanceData instance_data[ParticleEmitter::max_particle_count];
	glUseProgram(shader.shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, particle_emitter.texture_diffuse);

	// populate instance vbo ( offset to instances) before drawing
	int particle_count = particle_emitter.get_count();


	const Particle* particle_array = particle_emitter.get_particle_array();
	for (int i = 0, j=0; i < ParticleEmitter::max_particle_count; i++)
	{
		if (particle_array[i].is_alive)
		{
			Mat4x4 look_at = particle_array[i].transform.get_lookat(camera.transform.position);
			instance_data[j].instance_transform = projection * view * particle_array[i].transform.get_transform() *look_at;
			instance_data[j].instance_transform(0,0) *= 6.f;
			instance_data[j].instance_transform(1,1) *= 6.f;
			instance_data[j].instance_transform(2,2) *= 6.f;
			//float _z = pow(instance_data[j].instance_transform(2, 3)*5,20);
			float t = particle_array[i].life_time / particle_array[i].start_life_time;
			instance_data[j].color = lerp(particle_array[i].start_color, particle_array[i].end_color, t);
			//instance_data[j].color.y = 1.f-t;
			j+=1;
		}
	}

	// sort particle instance data by depth
	qsort(instance_data, particle_count, sizeof(InstanceData), compare_Particles);

	glBindBuffer(GL_ARRAY_BUFFER, particle_emitter.vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(InstanceData) * particle_count, instance_data);
	
	glBindVertexArray(particle_emitter.vaos[0]);
	glUniformMatrix4fv(shader.uniform_loc.view_projection, 1, GL_FALSE, (projection ).data);
	glUniform1i(shader.uniform_loc.texture_diffuse, 0);

	glDepthMask(false);
	glDrawElementsInstanced(mode, 6, GL_UNSIGNED_INT, (void*)0, particle_count);
	glDepthMask(true);
}


void draw_entity(const Entity& entity, WorldData& world_data, const Mat4x4& projection, const Mat4x4& view, const Mat4x4& model, Shader& shader, GLfloat parallax_scale, GLenum mode)
{
	glUseProgram(shader.shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, entity.texture_diffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, entity.texture_normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, entity.texture_parallax);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, world_data.light_depth_map);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, entity.texture_specular);

	for (int i = 0; i < entity.vao_count; i++)
	{
		glBindVertexArray(entity.vaos[i]);
		glUniformMatrix4fv(shader.uniform_loc.model, 1, GL_FALSE, model.data);
		glUniformMatrix4fv(shader.uniform_loc.mvp, 1, GL_FALSE, (projection*view*model).data);
		glUniformMatrix4fv(shader.uniform_loc.light_mvp, 1, GL_FALSE, (world_data.light_view_proj*model).data);

		glUniform1i(shader.uniform_loc.texture_diffuse, 0);
		glUniform1i(shader.uniform_loc.texture_normal, 1);
		glUniform1i(shader.uniform_loc.texture_parallax, 2);
		glUniform1i(shader.uniform_loc.texture_shadow_depth, 3);
		glUniform1i(shader.uniform_loc.texture_specular, 4);
		glUniform3f(shader.uniform_loc.light_dir, world_data.light_dir.x, world_data.light_dir.y, world_data.light_dir.z);
		glUniform3f(shader.uniform_loc.camera_pos, world_data.camera_pos.x, world_data.camera_pos.y, world_data.camera_pos.z);
		glUniform1i(shader.uniform_loc.lighting_mode, world_data.lighting_mode);
		glUniform1f(shader.uniform_loc.parallax_scale, parallax_scale);
		GLuint loc_bias = glGetUniformLocation(shader.shader_program, "bias");
		glUniform1f(loc_bias, world_data.bias);
		GLuint loc_shadow_mode = glGetUniformLocation(shader.shader_program, "shadow_mode");
		glUniform1i(loc_shadow_mode, world_data.shadow_mode);

		glDrawElements(mode, entity.index_counts[i], GL_UNSIGNED_INT, (void*)0);
	}
}

void make_texture(const char* texture_path, GLuint* texture, GLint internal_format, bool flip)
{
	if (!texture_path)
		return;

	Int_32 width = 0;
	Int_32 height = 0;
	Int_32 comp = 0;


	stbi_set_flip_vertically_on_load(flip);
	stbi_uc* tex_data = stbi_load(texture_path, &width, &height, &comp, 0);
	if (!tex_data)
	{
		printf("Failed to load image: %s . skipped texture creation\n", texture_path);
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

