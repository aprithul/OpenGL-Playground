#version 330 core

layout(location=0) in vec3 position;
layout(location=3) in vec2 uv;

out vec2 UV;
void main()
{
	UV = uv;
	gl_Position = vec4(position.x, position.y, 0, 1);
}

