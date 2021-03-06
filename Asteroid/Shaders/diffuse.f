#version 330 core

uniform vec3 dir_light;
uniform sampler2D tex2d;
uniform sampler2D texNormal;
uniform sampler2D parallaxMap;
uniform int lighting_mode;
uniform vec3 camera_pos;
uniform float parallax_scale;
in vec2 _uv;

in vec3 _normal;
in vec3 frag_pos_ts;
in vec3 view_pos_ts;
in vec3 _view_dir;
in vec3 _light_dir;
flat in mat3 TBN;

in vec3 vert_color;
out vec4 frag_color;

vec3 get_diffuse_light(vec3 frag_normal)
{
	vec3 light_color = vec3(0.4,0.4,0.4);
	float diffuse_strength = 2.f;
	return clamp( dot(frag_normal, -dir_light), 0, 1.0) * diffuse_strength * light_color;// dir_light is from light position towards object
}

vec3 get_specular_light_blinn_phong(vec3 frag_normal)
{
	vec3 half_vec = normalize( -dir_light + _view_dir);
	
	float specular_power = 300.f;
	float specular_light = clamp(pow(clamp(dot(normalize(_normal), half_vec), 0, 1.0),specular_power),0,1.0) ;
	vec3 specular_color = vec3(1,1,1);
	return  specular_color * specular_light * float( dot(normalize(_normal),-dir_light) > 0.0);
}


vec3 get_specular_light_phong(vec3 frag_normal)
{
	vec3 reflection_dir = normalize(reflect(dir_light, frag_normal));
	float specular_power = 40.f;
	float specular_light = pow(clamp(dot(reflection_dir, _view_dir), 0, 1.0),specular_power) ;
	vec3 specular_color = vec3(1,1,1);
	return  specular_color * specular_light;
}

vec2 get_parallex_steep(vec3 viewDir, vec2 texCoords)
{
	// steep parallax
	// number of depth layers
	const int minLayer = 16;
	const int maxLayer = 64;
	const vec3 n = vec3(0,0,1);
	float t = max(dot(viewDir, n), 0);
    int numLayers = 20;//int(minLayer*t+ maxLayer*(1.0 - t));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * parallax_scale; 
    vec2 deltaTexCoords = P / numLayers;

	// get initial values
	vec2  currentTexCoords     = texCoords;
	float currentDepthMapValue = texture(parallaxMap, currentTexCoords).r;
  
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(parallaxMap, currentTexCoords).r;  
		// get depth of next layer
		currentLayerDepth += layerDepth;  
	}


	// occlusion mapping
	vec2 prevTexCoord = currentTexCoords + deltaTexCoords;
	float postDif = currentDepthMapValue - currentLayerDepth;
	float preDif = texture(parallaxMap, prevTexCoord).r - currentLayerDepth + layerDepth;
	// interpolation weight. The higher postDif is a part of the total dif, 
	// the more we move towards previous texCords. and vice versa
	t = postDif / (postDif - preDif); 
	
	currentTexCoords = currentTexCoords*(1.0-t) + (prevTexCoord)*t;

	return currentTexCoords;
}

vec2 get_parallex(vec3 viewDir, vec2 texCoords)
{
	//simple parallax
	float depth = texture(parallaxMap, texCoords).r;
	vec2 offset = viewDir.xy*depth * parallax_scale;
	return texCoords - offset;
}

void main()
{
	
	vec3 frag_normal = normalize(_normal);
	vec2 texco = _uv;
	

	if(lighting_mode == 2)
	{
		vec3 view_dir_ts = normalize(view_pos_ts - frag_pos_ts);
		texco =  get_parallex_steep(view_dir_ts, texco);
		//if(texco.x>1.0 || texco.x<0.0 || texco.y>1.0 || texco.y<0.0)
		//	discard;
		//float h = texture(parallaxMap, texco).r;
		//frag_color = vec4(view_dir_ts*2-1,1);
		//return;
	}

	if(lighting_mode >= 1)
	{	
		frag_normal = normalize(texture(texNormal, texco).xyz * 2.0 - 1.0);
		frag_normal = normalize(TBN * frag_normal);
	}

	vec3 ambient_color = vec3(1,1,1)*0.4f;
	vec3 specular_color = get_specular_light_blinn_phong(frag_normal);
	vec3 diffuse_color = get_diffuse_light(frag_normal) + ambient_color; 
	frag_color = texture(tex2d, texco) * (vec4(diffuse_color,1) + vec4(specular_color,1));
}