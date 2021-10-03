#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in mat4 offset;
layout(location=6) in vec4 color;


uniform mat4 view_projection;

out vec4 tint_color;
out vec2 _uv;

void main()
{
	gl_Position = offset*vec4(position,1);//vec4(position + offset, 1);
	tint_color = color;
	_uv = uv;
}

