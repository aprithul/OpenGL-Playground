#version 330 core

uniform vec3 dir_light;
uniform sampler2D tex2d;
in vec2 _uv;
in vec3 _normal;
in vec3 _view_dir;
in vec3 vert_color;
out vec4 frag_color;

vec3 get_diffuse_light()
{
	vec3 light_color = vec3(0, 0, 0.5);
	return clamp( dot(_normal, -dir_light), 0, 1.0) * light_color;// dir_light is from light position towards object
}

vec3 get_specular_light()
{
	vec3 light = vec3(0,-1,-1);
	light = normalize(light);
	vec3 reflection_dir = normalize(reflect(dir_light, _normal));
	float specular_power = 200.f;
	float specular_light = pow(clamp(dot(reflection_dir, _view_dir), 0, 1.0),specular_power) ;
	vec3 specular_color = vec3(1,1,1);
	return  specular_color * specular_light;
}


void main()
{
	vec3 ambient_color = vec3(0.1, 0.1, 0.3);
	vec3 specular_color = get_specular_light();
	vec3 diffuse_color = get_diffuse_light(); 

	vec3 light = ambient_color + diffuse_color + specular_color;
	//frag_color = texture2D(tex2d, _uv) * light;
	frag_color = texture2D(tex2d, _uv) + vec4(light,1); 
}