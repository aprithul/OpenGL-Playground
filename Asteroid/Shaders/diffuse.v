#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 uv;

uniform mat4 model;
uniform mat4 mvp;
uniform vec3 camera_pos;
out vec3 vert_color;
out vec2 _uv;
out vec3 _normal;
out vec3 _view_dir;

void main()
{
	_uv = uv;
	_normal = normal;
	gl_Position = mvp * vec4(position.xyz,1);
	_view_dir = normalize(camera_pos - vec3(model * vec4(position, 1)));
	vert_color = color;
}