#version 330

layout(location=0) in vec3 position;

uniform mat4x4 mvp;

int main()
{
	gl_Position = mvp * position;	
}