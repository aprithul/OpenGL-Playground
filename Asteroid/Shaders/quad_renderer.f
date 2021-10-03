#version 330 core

uniform sampler2D tex2d;
in vec2 UV;
out vec4 frag_color;


void main()
{
	frag_color = texture(tex2d,UV);
}

