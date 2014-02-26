#version 430
//Change extension to glsl for nsight debugging
layout(early_fragment_tests) in;
in vec3 position;
in vec3 normal;
in vec2 uv;
in int instanceID;
out vec4 fragment_color;

uniform sampler2D diffuseTexture;
uniform bool renderNormals;

vec4 sample_color;

//vec2 lod;

void main()
{

	sample_color = texture(diffuseTexture, uv);

	//Alpha contrast by LOD for better transparency AA.
	//lod = textureQueryLod(diffuseTexture, uv);
	//if(lod.y < 0)
	//{
	//	sample_color.a = abs(clamp(lod.y, -100.0, -1.0)) * (1.0 + 1 * abs(clamp(lod.y, -1.0, 0.0))) * (sample_color.a - 0.5) + 0.5;
	//}
	//else if(lod.y >= 0.0 && sample_color.a > 00 && sample_color.a < 1.0)
	//{
	//	 sample_color.a = abs(clamp(lod.y, 1.0, 100.0)) * 0.85 * (sample_color.a - 0.5) + 0.5;
	//}

	if(renderNormals)
	{
		fragment_color = vec4(((normal + 1.0) / 2.0), sample_color.a);
	}
	else
	{
		fragment_color = vec4(sample_color.rgb, sample_color.a);
	}

	if(fragment_color.a < 0.5)
	{
		discard;
	}
}