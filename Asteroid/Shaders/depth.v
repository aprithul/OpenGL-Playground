#version 330 core

layout(location=0) in vec3 position;
layout(location=3) in vec2 uv;

uniform mat4 mvp;
out vec2 _uv;

void main()
{
	_uv = uv;
	gl_Position = mvp * vec4(position.xyz,1);
}