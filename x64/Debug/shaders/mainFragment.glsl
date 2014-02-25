#version 430
//Change extension to glsl for nsight debugging
//layout(early_fragment_tests) in;
in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 uv;
out vec4 fragment_color;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
//uniform sampler2D specularTexture;
uniform sampler3D voxelOcclusionTexture;
uniform sampler3D lightVoxelTexture;

uniform bool useAmbientOcclusion;
uniform bool useAtmosphericOcclusion;
uniform bool useGI;
uniform bool useTexture;

uniform uint voxelResolution;
uniform int shiftX;
uniform int shiftY;
uniform int shiftZ;

const float voxelStep = 1.0 / float(voxelResolution);
//const vec3 aoXTransform = vec3(voxelStep, 0.0, 0.0) * 2.0;
//const vec3 aoYTransform = vec3(0.0, voxelStep, 0.0) * 2.0;
//const vec3 aoZTransform = vec3(0.0, 0.0, voxelStep) * 2.0;

uniform mat4 model;

vec4 sample_color;

vec2 lod;

//vec3 lightposition;
vec3 intensity;

vec3 atmosphericOcclusion;

vec4 voxelSpace;
vec3 voxelSample;

vec3 aoValue;
vec3 aoSampleVectors[4];
vec3 aoSamples[5];
vec3 aoSample;
vec3 aoVector;
const float aoScalar[3] = float[3](1.0, 2.0, 3.0);

vec3 giSample;
vec3 giOcclusion;

vec3 atmospherePosition = vec3(0.0, 1.0, 0.0);
vec3 atmosphereColor = vec3(0.8196078431, 0.8901960784, 0.9725490196);

void combineIntensity(in vec3 vectorIntensity, out vec3 intensity)
{
	intensity = vec3(vectorIntensity.x + vectorIntensity.y + vectorIntensity.z);
}

void main()
{
	//position += (vec3(float(shiftX), float(shiftY), float(shiftX)) * vec3(voxelStep));
	sample_color = texture(diffuseTexture, uv);
	if(!useTexture)
	{
		sample_color = vec4(1.0, 1.0, 1.0, sample_color.a);
	}

	//sample_color = vec4(((normal + 1.0) / 2.0), sample_color.a);

	voxelSpace = ((model * vec4(position, 1.0)) + 0.5) + vec4(vec3(float(shiftX), float(shiftY), float(shiftZ)) * (voxelStep / 4.0), 1.0);

	intensity = ((normal + 1) * 0.5) * atmospherePosition;
	combineIntensity(intensity, intensity);
	//intensity = ((intensity * atmosphereColor) * 0.5) + 0.25;
	intensity = intensity * atmosphereColor;

	//voxelSample = textureLod(voxelOcclusionTexture, voxelSpace.xyz, 0).rgb;
	////sample_color = vec4(0.0, 0.0, 0.0, sample_color.a);
	//sample_color = sample_color + vec4(voxelSample.rgb, sample_color.a);

	if(useAtmosphericOcclusion)
	{
		atmosphericOcclusion = textureLod(voxelOcclusionTexture, voxelSpace.xyz, 1.50).rgb * 2.0;
		atmosphericOcclusion += textureLod(voxelOcclusionTexture, voxelSpace.xyz, 2.50).rgb * 5.0;
		atmosphericOcclusion += textureLod(voxelOcclusionTexture, voxelSpace.xyz, 3.50).rgb * 7.0;
		clamp(atmosphericOcclusion, 0.0, 1.0);
		//normalize(atmosphericOcclusion);
		atmosphericOcclusion *= abs(normal);
		combineIntensity(atmosphericOcclusion, atmosphericOcclusion);

		intensity = atmosphereColor * (atmosphericOcclusion / 12.0);
	}

	aoValue = vec3(0.0f, 0.0f, 0.0f);

	if(useAmbientOcclusion)
	{
		//aoSampleVectors[0] = ((normal + tangent) / 2.0) + (tangent * voxelStep * 0.5);
		//aoSampleVectors[1] = ((normal + (-tangent)) / 2.0) - (tangent * voxelStep * 0.5);
		//aoSampleVectors[2] = ((normal + bitangent) / 2.0) + (bitangent * voxelStep * 0.5);
		//aoSampleVectors[3] = ((normal + (-bitangent)) / 2.0) - (bitangent * voxelStep * 0.5);

		//aoSampleVectors[0] = (normal + normal.xzy) / 2.0;
		//aoSampleVectors[1] = (normal + -normal.xzy) / 2.0;
		//aoSampleVectors[2] = (normal + normal.yxz) / 2.0;
		//aoSampleVectors[3] = (normal + -normal.yxz) / 2.0;

		//aoSamples[0] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (aoSampleVectors[0] * voxelStep * 1.5), 1).rgb;// * abs(aoSampleVectors[0]) * 2.0;
		//aoSamples[0] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (aoSampleVectors[0] * 2.0 * voxelStep), 0.5).rgb;// * abs(aoSampleVectors[0]) * 2.0;
		//aoSamples[0] /= 2.0;
		//clamp(aoSamples[0], 0.0, 1.0);
		//combineIntensity(aoSamples[0], aoSamples[0]);
		//aoSamples[1] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (aoSampleVectors[1] * voxelStep * 1.5), 1).rgb;// * abs(aoSampleVectors[1]) * 2.0;
		//aoSamples[1] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (aoSampleVectors[1] * 2.0 * voxelStep), 0.5).rgb;// * abs(aoSampleVectors[1]) * 2.0;
		//aoSamples[1] /= 2.0;
		//clamp(aoSamples[1], 0.0, 1.0);
		//combineIntensity(aoSamples[1], aoSamples[1]);
		//aoSamples[2] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (aoSampleVectors[2] * voxelStep * 1.5), 1).rgb;// * abs(aoSampleVectors[2]) * 2.0;
		//aoSamples[2] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (aoSampleVectors[2] * 2.0 * voxelStep), 0.5).rgb;// * abs(aoSampleVectors[2]) * 2.0;
		//aoSamples[2] /= 2.0;
		//clamp(aoSamples[2], 0.0, 1.0);
		//combineIntensity(aoSamples[2], aoSamples[2]);
		//aoSamples[3] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (aoSampleVectors[3] * voxelStep * 1.5), 1).rgb;// * abs(aoSampleVectors[3]) * 2.0;
		//aoSamples[3] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (aoSampleVectors[3] * 2.0 * voxelStep), 0.5).rgb;// * abs(aoSampleVectors[3]) * 2.0;
		//aoSamples[3] /= 2.0;
		//clamp(aoSamples[3], 0.0, 1.0);
		//combineIntensity(aoSamples[3], aoSamples[3]);
		//aoSamples[4] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 1.5), 1).rgb;// * abs(normal) * 2.0;
		//aoSamples[4] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * 2.0 * voxelStep), 0.5).rgb;// * abs(normal) * 2.0;
		//aoSamples[4] /= 2.0;
		//clamp(aoSamples[4], 0.0, 1.0);
		//combineIntensity(aoSamples[4], aoSamples[4]);


		aoSamples[0] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 2.0), 1).rgb * 2.0;// * abs(normal) * 2.0;
		aoSamples[0] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 1.0), 0.5).rgb * 1.0;// * abs(normal) * 2.0;
		aoSamples[0] /= 2.0;
		clamp(aoSamples[0], 0.0, 1.0);
		combineIntensity(aoSamples[0], aoSamples[0]);
		aoSamples[1] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 4.0), 2).rgb * 4.0;// * abs(normal) * 2.0;
		aoSamples[1] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 3.0), 1.5).rgb * 3.0;// * abs(normal) * 2.0;
		aoSamples[1] /= 2.0;
		clamp(aoSamples[1], 0.0, 1.0);
		combineIntensity(aoSamples[1], aoSamples[1]);
		aoSamples[2] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 6.0), 3).rgb * 6.0;// * abs(normal) * 2.0;
		aoSamples[2] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 5.0), 2.5).rgb * 5.0;// * abs(normal) * 2.0;
		aoSamples[2] /= 2.0;
		clamp(aoSamples[2], 0.0, 1.0);
		combineIntensity(aoSamples[2], aoSamples[2]);
		aoSamples[3] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 8.0), 4).rgb * 8.0;// * abs(normal) * 2.0;
		aoSamples[3] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 7.0), 3.5).rgb * 7.0;// * abs(normal) * 2.0;
		aoSamples[3] /= 2.0;
		clamp(aoSamples[3], 0.0, 1.0);
		combineIntensity(aoSamples[3], aoSamples[3]);
		aoSamples[4] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 10.0), 5).rgb * 10.0;// * abs(normal) * 2.0;
		aoSamples[4] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 9.0), 4.5).rgb * 9.0;// * abs(normal) * 2.0;
		aoSamples[4] /= 2.0;
		clamp(aoSamples[4], 0.0, 1.0);
		combineIntensity(aoSamples[4], aoSamples[4]);






		//aoValue = aoSamples[4];
		aoValue = ((aoSamples[0] * 0.25) + (aoSamples[1] * 0.25) + (aoSamples[2] * 0.25) + (aoSamples[3] * 0.25) + (aoSamples[4] * 0.25));


	}

	intensity = intensity - (aoValue * 0.15);


	giSample = vec3(0.0);

	if(useGI)
	{
		giSample = (textureLod(lightVoxelTexture, voxelSpace.xyz, 0.5).rgb + (textureLod(lightVoxelTexture, voxelSpace.xyz, 1.5).rgb * 3.0)) / 2.0;
		giSample += textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 1.0), 0.5).rgb * 1.0;
		giOcclusion = clamp((textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 1.0), 0.5).rgb * 1.0), 0.0, 1.0);
		giSample += (textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 3.0), 1.5).rgb * 3.0) - giOcclusion;
		giOcclusion += clamp((textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 3.0), 1.5).rgb * 3.0), 0.0, 1.0);
		giSample += (textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 7.0), 2.5).rgb * 5.0) - giOcclusion;
		giOcclusion += clamp((textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 7.0), 2.5).rgb * 5.0), 0.0, 1.0);
		giSample += (textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 15.0), 3.5).rgb * 7.0) - giOcclusion;
		//giOcclusion += clamp((textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 15.0), 3.5).rgb * 7.0), 0.0, 1.0);
		//giSample += (textureLod(lightVoxelTexture, voxelSpace.xyz + (normal * voxelStep * 31.0), 4.5).rgb * 9.0) - giOcclusion;
		clamp(giSample, 0.0, 100.0);
	}






	intensity = intensity + giSample;


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


	//voxelSample = textureLod(voxelOcclusionTexture, voxelSpace.xyz, 0).rgb;
	//sample_color = vec4(0.0, 0.0, 0.0, sample_color.a);
	//sample_color = sample_color + vec4(voxelSample.rgb, sample_color.a);
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