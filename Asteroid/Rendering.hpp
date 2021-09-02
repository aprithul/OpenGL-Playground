#ifndef RENDERING_HPP
#define RENDERING_HPP

#include "stb_image.h"
#include "glew.h"
#include "Entity.hpp"
#include "SDL.h"

extern SDL_Window* window;
extern SDL_GLContext gl_context;

struct WorldData
{
	Vec3f light_dir;
	GLuint light_depth_map;
	Mat4x4 light_view;
	Vec3f camera_pos;
	float bias;
	int lighting_mode;
	int shadow_mode;
};

struct UniformLoc
{
	GLint mvp = 0;
	GLint model = 0;
	GLint texture_diffuse = 0;
	GLint texture_normal = 0;
	GLint texture_parallax = 0;
	GLint texture_shadow_depth = 0;
	GLint texture_specular = 0;
	GLint light_dir = 0;
	GLint camera_pos = 0;
	GLint lighting_mode = 0;
	GLint parallax_scale = 0;
	GLint light_mvp = 0;

};

struct Shader
{
	GLint shader_program;
	UniformLoc uniform_loc;

	Shader(const char* vertex_path, const char* fragment_path);
	~Shader();

	void load_uniform_locations()
	{
		uniform_loc.mvp = glGetUniformLocation(shader_program, "mvp");
		uniform_loc.light_mvp = glGetUniformLocation(shader_program, "light_mvp");
		uniform_loc.model = glGetUniformLocation(shader_program, "model");
		uniform_loc.texture_diffuse = glGetUniformLocation(shader_program, "tex2d");
		uniform_loc.texture_normal = glGetUniformLocation(shader_program, "texNormal");
		uniform_loc.texture_parallax = glGetUniformLocation(shader_program, "parallaxMap");
		uniform_loc.texture_specular = glGetUniformLocation(shader_program, "specularMap");
		uniform_loc.light_dir = glGetUniformLocation(shader_program, "dir_light");
		uniform_loc.camera_pos = glGetUniformLocation(shader_program, "camera_pos");
		uniform_loc.lighting_mode = glGetUniformLocation(shader_program, "lighting_mode");
		uniform_loc.parallax_scale = glGetUniformLocation(shader_program, "parallax_scale");
		uniform_loc.texture_shadow_depth = glGetUniformLocation(shader_program, "shadowDepthMap");
	}
};

int InitOpenglWindow(Int_32 w, Int_32 h);
GLuint make_depth_framebuffer(int w, int h, GLuint* depth_texture);
GLuint make_frame_buffer();
void draw_entity(const Entity& entity, WorldData& world_data, const Mat4x4& projection, const Mat4x4& view, const Mat4x4& model, Shader& shader, GLfloat parallax_scale, GLenum mode = GL_LINE_LOOP);
void make_mesh(Vertex* vertices, Int_32* indices, Int_32 vertex_count, Int_32 index_count, Uint_32& vao, int attrib_count, bool is_textured);
void make_texture(const char* texture_path, GLuint* texture, GLint internal_format, bool flip);

#endif // !RENDERING_HPP
