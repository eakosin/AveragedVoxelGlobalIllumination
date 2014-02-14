#version 430
//Change extension to glsl for nsight debugging
in vec3 position;
in vec3 normal;
in vec2 uv;
out vec4 fragment_color;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D opacityTexture;
uniform sampler3D voxelOcclusionTexture;

uniform vec3 center;
uniform float modelScale;
uniform uint voxelResolution;

uniform mat4 model;

vec4 sample_color;
vec3 intensity;

vec2 lod;

vec3 lightposition;

vec4 voxelSpace;
vec3 voxelSample;

void main()
{
	lod = textureQueryLod(diffuseTexture, uv);
	lightposition = vec3(0.0, 0.75, 0.5);
	intensity = ((normal + 1) / 2) * lightposition;
	intensity = vec3(intensity.x + intensity.y + intensity.z);
	sample_color = texture(diffuseTexture, uv);
	sample_color = vec4(0.0, 0.0, 0.0, sample_color.a);

	voxelSpace = 1.0 - ((model * vec4(position, 1.0)) + 0.5);
	//voxelSpace = (position * 0.000268756) - center + 0.5;

	voxelSample = textureLod(voxelOcclusionTexture, voxelSpace.xyz, 0).rgb;
	sample_color = sample_color + vec4(voxelSample, sample_color.a);




	//Alpha contrast by LOD for better transparency AA.
	if(lod.y < 0)
	{
		sample_color.a = abs(clamp(lod.y, -100.0, -1.0)) * (1.0 + 1 * abs(clamp(lod.y, -1.0, 0.0))) * (sample_color.a - 0.5) + 0.5;
	}
	else if(lod.y >= 0.0 && sample_color.a > 00 && sample_color.a < 1.0)
	{
		 sample_color.a = abs(clamp(lod.y, 1.0, 100.0)) * 0.85 * (sample_color.a - 0.5) + 0.5;
	}
	fragment_color = vec4(sample_color.rgb * intensity, sample_color.a);
	//if(fragment_color.a < 0.5)
	//{
	//	discard;
	//}
}