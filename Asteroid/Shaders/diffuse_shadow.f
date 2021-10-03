#version 330 core

uniform sampler2D tex2d;
uniform sampler2D texNormal;
uniform sampler2D parallaxMap;
uniform sampler2D shadowDepthMap;
uniform sampler2D specularMap;
uniform int lighting_mode;
uniform vec3 camera_pos;
uniform float parallax_scale;
uniform float bias;
uniform int shadow_mode;

in vec3 vert_color;
in vec2 _uv;
in vec3 frag_pos_ts;
in vec3 view_pos_ts;
in vec3 _normal;
in vec4 light_space_position;
in vec3 dir_light_ts;
in vec3 dir_view_ts;
in vec3 dir_light_ws;
in vec3 dir_view_ws;

out vec4 frag_color;

//flat in mat3 TBN;

//in vec3 vert_color;

vec3 get_diffuse_light(vec3 frag_normal, vec3 light_direction)
{
	vec3 light_color = vec3(0.4,0.4,0.4);
	float diffuse_strength = 2.f;
	return clamp( dot(frag_normal, light_direction), 0, 1.0) * diffuse_strength * light_color;// dir_light_ts is  from object to light position
}

vec3 get_specular_light_blinn_phong(vec3 frag_normal, vec3 light_direction, vec3 view_direction, vec2 _uv)
{
	vec3 half_vec = normalize( light_direction + view_direction);
	
	float specular_power = 200.f;
	float specular_light = clamp(pow(clamp(dot(normalize(frag_normal), half_vec), 0, 1.0),specular_power),0,1.0) * texture(specularMap,_uv).r;
	vec3 specular_color = vec3(1,1,1);
	return  specular_color * specular_light * float( dot(normalize(frag_normal),light_direction) > 0.0);
}

vec2 get_parallex_steep(vec3 viewDir, vec2 texCoords)
{
	// steep parallax
	// number of depth layers
	const int minLayer = 16;
	const int maxLayer = 64;
	const vec3 n = vec3(0,0,1);
	//float t = max(dot(viewDir, n), 0);
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
	float t = postDif / (postDif - preDif); 
	
	currentTexCoords = currentTexCoords*(1.0-t) + (prevTexCoord)*t;

	return currentTexCoords;
}

float get_penumbra_size(float light_width)
{
	vec3 proj_texco = light_space_position.xyz / light_space_position.w;
	proj_texco = light_space_position.xyz*0.5+0.5;
	float texel_size = 1.f/textureSize(shadowDepthMap,0).x;
	int samples = 33;

	float half_dim = (samples*texel_size)/2.f;
	float depth_total = 0;
	float depth_count = 0;
	for(int x = -samples/2; x <= samples/2; x++)
	{
		for(int y = -samples/2; y <= samples/2; y++)
		{
			float _depth = texture(shadowDepthMap, proj_texco.xy + (vec2(x,y)*texel_size)).r;

			if(_depth < proj_texco.z-0.005f)
			{
				depth_total += _depth;
				depth_count++;
			}
		}
	}
	
	//return texture(shadowDepthMap, proj_texco.xy).r / proj_texco.z;
	
	if(depth_count == 0)
		return 0;

	float avg_distance = depth_total / depth_count;
	return (proj_texco.z - avg_distance)/avg_distance;
	//return penumbra;
}

vec3 get_proj()
{
	vec3 proj_texco = light_space_position.xyz;// / light_space_position.w;
	proj_texco = light_space_position.xyz*0.5+0.5;
	float _depth = pow(texture(shadowDepthMap, proj_texco.xy).r,15);
	return vec3(_depth,_depth,_depth);
}

float get_shadow_factor()
{
	
	vec3 proj_texco = light_space_position.xyz/ light_space_position.w;
	proj_texco = proj_texco.xyz*0.5+0.5;
	float texel_size = 1.f/textureSize(shadowDepthMap,0).x;
	float penumbra = get_penumbra_size(1);
	
	if(shadow_mode == 0) // no shadow
		return 0;

	int samples = 0;
	if(shadow_mode == 1) // hard shadow
	{
		samples = 1;
		float _depth = texture(shadowDepthMap, proj_texco.xy).r;
		return _depth + max(bias,0) < proj_texco.z ? 1 :0;
	}
	else if(shadow_mode == 2) // soft shadow
		samples = 7;	
	else if(shadow_mode == 3) // PCSS shadow
	{
		samples = 1 + int(penumbra*33);
		if(samples%2==0)	// even number of samples cause artifact
			samples+=1;
	}

	float half_dim = (samples*texel_size)/2.f;
	float shadow_total = 0;
	for(int x = -samples/2; x <= samples/2; x++)
	{
		for(int y = -samples/2; y <= samples/2; y++)
		{

			float _depth = texture(shadowDepthMap, proj_texco.xy + (vec2(x,y)*texel_size)).r;
			shadow_total += _depth + max(bias,0) < proj_texco.z ? 1 :0;
		}
	}
	
	return shadow_total / (samples*samples);
}


float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

	float pcfDepth = texture(shadowDepthMap, projCoords.xy).r; 
    float shadow = projCoords.z - bias > pcfDepth  ? 1.0 : 0.0;        
	return shadow;

	//return projCoords.z;
    float closestDepth = texture(shadowDepthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowDepthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowDepthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
	
	vec3 frag_normal = normalize(_normal);
	vec3 light_direction = normalize(dir_light_ws);
	vec3 view_direction = normalize(dir_view_ws);
	vec2 texco = _uv;
	vec3 ambient_color = vec3(0,0,0);
	vec3 specular_color = vec3(0,0,0);
	vec3 diffuse_color = vec3(1,1,1);
	float shadow_strength = 0;
	float shadow_factor = 1;


	//if(lighting_mode >=4)
	//{
	shadow_strength = 0.75f;
	shadow_factor = 1.f - ((get_shadow_factor())*shadow_strength);
	//}

	if(lighting_mode >= 3)
	{
		light_direction = dir_light_ts;
		view_direction = dir_view_ts;
		texco =  get_parallex_steep(view_direction, texco);
	}

	if(lighting_mode >= 2)
	{	
		frag_normal = normalize(normalize(texture(texNormal, texco).xyz) * 2.0 - 1.0);
		light_direction = dir_light_ts;
		view_direction = dir_view_ts;
		//frag_normal = normalize(TBN * frag_normal);
	}

	if(lighting_mode >= 1 )
	{
		ambient_color = vec3(1,1,1)*0.4f;
		specular_color = get_specular_light_blinn_phong(frag_normal, light_direction, view_direction, texco);
		diffuse_color = get_diffuse_light(frag_normal, light_direction) + ambient_color; 
	}
	
	//float d = get_depth();
	vec4 sampled_color = texture(tex2d, texco);
	//vec4 sampled_color = vec4(p,p,p,1);

	//float frensel_factor = dot(frag_normal, dir_view_ws);
	//frensel_factor = smoothstep(0.1f, 0.5f, frensel_factor);

	//frag_color = (sampled_color * shadow_factor * (vec4(diffuse_color,1) + vec4(specular_color,1))) + (vec4(1,1,1,1)*(1.f-frensel_factor));
	frag_color = vec4((sampled_color * shadow_factor * (vec4(diffuse_color,1) + vec4(specular_color,1))).xyz,1);// + (vec4(1,1,1,1)*(1.f-frensel_factor));
}