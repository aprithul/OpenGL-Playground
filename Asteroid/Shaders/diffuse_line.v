#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;

uniform mat4 mvp;
out vec3 vert_color;

void main()
{
	gl_Position = mvp * vec4(position.xyz,1);
	vert_color = color;
}