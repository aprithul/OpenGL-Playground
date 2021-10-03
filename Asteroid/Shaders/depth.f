#version 330 core

uniform sampler2D tex2d; 
in vec2 _uv;
out vec4 frag_color;

float LinearizeDepth(float depth)
{
	float far_plane = 10.f;
	float near_plane = 0.1f;

    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
	float _d = pow(texture(tex2d, _uv).r,10.f);
	frag_color = vec4(_d, _d, _d, 1);
}