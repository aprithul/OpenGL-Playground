#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 uv;

uniform mat4 mvp;
out vec3 vert_color;

void main()
{
	gl_Position = mvp * vec4(position.xyz,1);
	vert_color = color;
}