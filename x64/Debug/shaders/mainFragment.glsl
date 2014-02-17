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

vec2 lod;

//vec3 lightposition;
vec3 intensity;
vec3 intensityS1;

vec4 voxelSpace;
vec3 voxelSampleS1[4];
vec3 voxelSampleS2[4];

vec3 atmospherePosition = vec3(0.0, 1.0, 0.0);

void combineIntensity(in vec3 vectorIntensity, out vec3 intensity)
{
	intensity = vec3(vectorIntensity.x + vectorIntensity.y + vectorIntensity.z);
}

void main()
{
	sample_color = texture(diffuseTexture, uv);

	voxelSpace = ((model * vec4(position, 1.0)) + 0.5);

	//voxelSample = textureLod(voxelOcclusionTexture, voxelSpace.xyz, 0).rgb;
	//sample_color = vec4(0.0, 0.0, 0.0, sample_color.a);
	//sample_color = sample_color + vec4(voxelSample.rgb, sample_color.a);

	intensity = ((normal + 1) * 0.5) * atmospherePosition;
	combineIntensity(intensity, intensity);
	intensity = ((intensity * vec3(0.8196078431, 0.8901960784, 0.9725490196)) * 0.5) + 0.25;
	voxelSampleS1[0] = textureLod(voxelOcclusionTexture, vec3(voxelSpace.x + float(1 / voxelResolution), voxelSpace.y + float(1 / (voxelResolution * 1)),voxelSpace.z + float(1 / voxelResolution)) , 1).rgb * vec3(0.25, 1.0, 0.25);
	combineIntensity(voxelSampleS1[0], voxelSampleS1[0]);
	voxelSampleS1[1] = textureLod(voxelOcclusionTexture, vec3(voxelSpace.x - float(1 / voxelResolution), voxelSpace.y + float(1 / (voxelResolution * 1)),voxelSpace.z + float(1 / voxelResolution)), 1).rgb * vec3(0.25, 1.0, 0.25);
	combineIntensity(voxelSampleS1[1], voxelSampleS1[1]);
	voxelSampleS1[2] = textureLod(voxelOcclusionTexture, vec3(voxelSpace.x + float(1 / voxelResolution), voxelSpace.y + float(1 / (voxelResolution * 1)),voxelSpace.z - float(1 / voxelResolution)), 1).rgb * vec3(0.25, 1.0, 0.25);
	combineIntensity(voxelSampleS1[2], voxelSampleS1[2]);
	voxelSampleS1[3] = textureLod(voxelOcclusionTexture, vec3(voxelSpace.x - float(1 / voxelResolution), voxelSpace.y + float(1 / (voxelResolution * 1)),voxelSpace.z - float(1 / voxelResolution)), 1).rgb * vec3(0.25, 1.0, 0.25);
	combineIntensity(voxelSampleS1[3], voxelSampleS1[3]);
	intensityS1 = (voxelSampleS1[0] + voxelSampleS1[1] + voxelSampleS1[2] + voxelSampleS1[3]) / 4;

	intensity = intensity - intensityS1;


	//Alpha contrast by LOD for better transparency AA.
	lod = textureQueryLod(diffuseTexture, uv);
	if(lod.y < 0)
	{
		sample_color.a = abs(clamp(lod.y, -100.0, -1.0)) * (1.0 + 1 * abs(clamp(lod.y, -1.0, 0.0))) * (sample_color.a - 0.5) + 0.5;
	}
	else if(lod.y >= 0.0 && sample_color.a > 00 && sample_color.a < 1.0)
	{
		 sample_color.a = abs(clamp(lod.y, 1.0, 100.0)) * 0.85 * (sample_color.a - 0.5) + 0.5;
	}

	//fragment_color = vec4(sample_color.rgb, sample_color.a);

	//lightposition = vec3(0.0, 0.75, 0.5);
	//intensity = ((normal + 1) / 2) * lightposition;
	//intensity = vec3(intensity.x + intensity.y + intensity.z);
	fragment_color = vec4(sample_color.rgb * intensity, sample_color.a);
	//if(fragment_color.a < 0.5)
	//{
	//	discard;
	//}
}