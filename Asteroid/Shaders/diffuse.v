#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 uv;
layout(location=4) in vec3 tangent;

uniform mat4 model;
uniform mat4 mvp;
uniform vec3 camera_pos;
out vec3 vert_color;
out vec2 _uv;
out vec3 frag_pos_ts;
out vec3 view_pos_ts;
out vec3 _normal;


out vec3 _view_dir;
out vec3 _light_dir;

flat out mat3 TBN;

void main()
{
	vec3 light_pos = vec3(0,5,.1);
	_uv = uv;
	vec3 _bitangent = cross( normal, tangent);

	_bitangent = normalize( mat3(model) * _bitangent);
	_normal = normalize( mat3(model) * normal);
	vec3 _tangent = normalize( mat3(model) * tangent);

	TBN = mat3(_tangent, _bitangent, _normal);
	mat3 TBN_t = transpose(TBN);

	gl_Position = mvp * vec4(position.xyz,1);
	frag_pos_ts = TBN_t * vec3(model * vec4(position.xyz,1));
	view_pos_ts = TBN_t * camera_pos;

	_view_dir = normalize(camera_pos - vec3(model * vec4(position, 1)));
	_light_dir = normalize(light_pos - vec3(model * vec4(position, 1)));
	vert_color = color;
}