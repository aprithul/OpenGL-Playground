#version 330 core

uniform sampler2D tex2d;
in vec2 UV;
out vec4 frag_color;


void main()
{
	vec4 outline_color = vec4(0,0.3f,0.5f,1);
	vec2 texel_size = 1.f/textureSize(tex2d, 0);
	vec3 sum = vec3(0,0,0);
	int samples = 9;
	for(int i=-samples/2; i<=samples/2; i++)
	{
		for(int j=-samples/2; j<=samples/2; j++)
		{
			sum += texture(tex2d, UV+vec2(i*texel_size.x,j*texel_size.y)).rgb;
		}
	}

	//if(sum.g>0 && sum.g <= outline_color.g*samples*samples/3)	// only outline through opeque geometry, condition depends specifically on outline color value
	if(sum.g>0)	// silhouette through opeque, condition depends specifically on outline color value
		frag_color = vec4(0,0.3f,0.5f,1);//(sum/(samples*samples),1);
	else
		frag_color = vec4(0,0,0,0);
}