#version 330 core

uniform sampler2D tex2d;

in vec4 tint_color;
in vec2 _uv;
out vec4 frag_color;

void main()
{
	frag_color = texture(tex2d, _uv) * tint_color;
}