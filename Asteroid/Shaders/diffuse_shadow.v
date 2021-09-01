#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 uv;
layout(location=4) in vec3 tangent;

uniform vec3 dir_light;
uniform mat4 model;
uniform mat4 mvp;
uniform mat4 light_mvp;
uniform vec3 camera_pos;


out vec3 vert_color;
out vec2 _uv;
//out vec3 frag_pos_ts;
//out vec3 view_pos_ts;
out vec3 _normal;
out vec4 light_space_position;
out vec3 dir_light_ts;
out vec3 dir_view_ts;
out vec3 dir_light_ws;
out vec3 dir_view_ws;

//flat out mat3 TBN;

void main()
{
	_uv = uv;
	vec3 _bitangent = cross( normal, tangent);

	_bitangent = normalize( mat3(model) * _bitangent);
	_normal = normalize( mat3(model) * normal);
	vec3 _tangent = normalize( mat3(model) * tangent);

	mat3 TBN_t = transpose(mat3(_tangent, _bitangent, _normal));
	//mat3 TBN_t = transpose(TBN);

	gl_Position = mvp * vec4(position.xyz,1);
	light_space_position = light_mvp * vec4(position.xyz,1);
	//light_space_position = vec3(light_space_position_pre_perspective_divide.xyz/light_space_position_pre_perspective_divide.w);

	//frag_pos_ts = TBN_t * vec3(model * vec4(position.xyz,1));
	//view_pos_ts = TBN_t * camera_pos;
	dir_light_ws = -dir_light;
	dir_light_ts = TBN_t * dir_light_ws;
	dir_view_ws = normalize(camera_pos - gl_Position.xyz);
	dir_view_ts = TBN_t * dir_view_ws;
	vert_color = color;
}