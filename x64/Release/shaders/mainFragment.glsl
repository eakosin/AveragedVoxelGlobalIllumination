// Averaged Voxel GLobal Illumination
// Copyright (C) 2014  Evan Arthur Kosin
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#version 430
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
uniform sampler3D lightNormalTexture;

uniform bool useAmbientOcclusion;
uniform bool useAtmosphericOcclusion;
uniform bool useGI;
uniform bool useTexture;

uniform uint voxelResolution;
uniform uint giResolution;
uniform int shiftX;
uniform int shiftY;
uniform int shiftZ;

uniform mat4 model;


const float voxelStep = 1.0 / float(voxelResolution);
const float giStep = 1.0 / float(giResolution);
const float voxelRatio = float(voxelResolution) / float(giResolution);


vec4 sample_color;

vec2 lod;

vec3 intensity;

vec3 atmosphericOcclusion;

vec4 voxelSpace;
vec3 voxelSample;

vec3 aoValue;
vec3 aoSampleVectors[4];
vec3 aoSamples[5];
vec3 aoVector;
const float aoScalar[3] = float[3](1.0, 2.0, 3.0);

vec3 giSample;
vec3 giTempSample;
vec3 giOcclusion;
vec4 giSpace;
vec3 giNormal;

vec3 atmospherePosition = vec3(0.0, 1.0, 0.0);
vec3 atmosphereColor = vec3(0.8196078431, 0.8901960784, 0.9725490196);

// Add xyz values of vec3 and return vec3 of this result
vec3 combineIntensity(in vec3 vectorIntensity)
{
	return vec3(vectorIntensity.x + vectorIntensity.y + vectorIntensity.z);
}

void main()
{
	// Sample texture
	sample_color = texture(diffuseTexture, uv);
	//if(sample_color.a < 0.5)
	//{
	//	discard;
	//}

	// Replace texture color with white for visualization
	if(!useTexture)
	{
		sample_color = vec4(1.0, 1.0, 1.0, sample_color.a);
	}

	// Convert fragment position to voxelspace of 0.0 to 1.0
	voxelSpace = ((model * vec4(position, 1.0)) + 0.5) + vec4(vec3(float(shiftX), float(shiftY), float(shiftZ)) * (voxelStep * 0.25), 1.0);

	// Calculate initial lighting value.
	intensity = ((normal + 1) * 0.5) * atmospherePosition;
	intensity = combineIntensity(intensity);
	intensity = intensity * atmosphereColor;

	// Visualize voxel data on scene geometry.
	//voxelSample = textureLod(voxelOcclusionTexture, voxelSpace.xyz, 0).rgb;
	////sample_color = vec4(0.0, 0.0, 0.0, sample_color.a);
	//sample_color = sample_color + vec4(voxelSample.rgb, sample_color.a);

	// Calculate occlusion from top of scene by sampling voxelOcclusionTexture repeatedly
	if(useAtmosphericOcclusion)
	{
		atmosphericOcclusion = textureLod(voxelOcclusionTexture, voxelSpace.xyz + vec3(0.0, voxelStep * 4.0, 0.0), 2.0).rgb * 4.0;
		atmosphericOcclusion += textureLod(voxelOcclusionTexture, voxelSpace.xyz + vec3(0.0, (voxelStep * 6.0) * 2.0, 0.0), 3.0).rgb * 5.0;
		atmosphericOcclusion += textureLod(voxelOcclusionTexture, voxelSpace.xyz + vec3(0.0, (voxelStep * 8.0) * 3.0, 0.0), 4.0).rgb * 6.0;
		atmosphericOcclusion += textureLod(voxelOcclusionTexture, voxelSpace.xyz + vec3(0.0, (voxelStep * 10.0) * 4.0, 0.0), 5.0).rgb * 5.0;

		atmosphericOcclusion = combineIntensity(atmosphericOcclusion);
		atmosphericOcclusion = clamp(atmosphericOcclusion, 0.0, 1.0);

		intensity = intensity * (1.0 - (atmosphericOcclusion * 0.5));
	}

	aoValue = vec3(0.0f, 0.0f, 0.0f);

	// Calculate Ambient Occlusion by sampling one step outward from surface normal for several mip-map layers
	// and combining
	if(useAmbientOcclusion)
	{
		//aoSampleVectors[0] = ((normal + tangent) * 0.5) + (tangent * voxelStep * 0.5);
		//aoSampleVectors[1] = ((normal + (-tangent)) * 0.5) - (tangent * voxelStep * 0.5);
		//aoSampleVectors[2] = ((normal + bitangent) * 0.5) + (bitangent * voxelStep * 0.5);
		//aoSampleVectors[3] = ((normal + (-bitangent)) * 0.5) - (bitangent * voxelStep * 0.5);

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
		aoSamples[0] *= 0.5;
		clamp(aoSamples[0], 0.0, 1.0);
		aoSamples[0] = combineIntensity(aoSamples[0]);
		aoSamples[1] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 4.0), 2).rgb * 4.0;// * abs(normal) * 2.0;
		aoSamples[1] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 3.0), 1.5).rgb * 3.0;// * abs(normal) * 2.0;
		aoSamples[1] *= 0.5;
		clamp(aoSamples[1], 0.0, 1.0);
		aoSamples[1] = combineIntensity(aoSamples[1]);
		aoSamples[2] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 6.0), 3).rgb * 6.0;// * abs(normal) * 2.0;
		aoSamples[2] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 5.0), 2.5).rgb * 5.0;// * abs(normal) * 2.0;
		aoSamples[2] *= 0.5;
		clamp(aoSamples[2], 0.0, 1.0);
		aoSamples[2] = combineIntensity(aoSamples[2]);
		aoSamples[3] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 8.0), 4).rgb * 8.0;// * abs(normal) * 2.0;
		aoSamples[3] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 7.0), 3.5).rgb * 7.0;// * abs(normal) * 2.0;
		aoSamples[3] *= 0.5;
		clamp(aoSamples[3], 0.0, 1.0);
		aoSamples[3] = combineIntensity(aoSamples[3]);
		aoSamples[4] = textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 10.0), 5).rgb * 10.0;// * abs(normal) * 2.0;
		aoSamples[4] += textureLod(voxelOcclusionTexture, voxelSpace.xyz + (normal * voxelStep * 9.0), 4.5).rgb * 9.0;// * abs(normal) * 2.0;
		aoSamples[4] *= 0.5;
		clamp(aoSamples[4], 0.0, 1.0);
		aoSamples[4] = combineIntensity(aoSamples[4]);

		aoValue = ((aoSamples[0] * 0.25) + (aoSamples[1] * 0.25) + (aoSamples[2] * 0.25) + (aoSamples[3] * 0.25) + (aoSamples[4] * 0.25));
	}

	intensity = intensity - (aoValue * 0.05);


	giSample = vec3(0.0);

	// Calculate Global Illumination by stepping out repeatedly along the 3D texture in increasing mip-map levels, accumulating lighting attenuated
	// by the direction of the step minus the lighting normal and scaling by the accumulated occlusion of previous voxels that were sampled
	if(useGI)
	{
		giSpace = ((model * vec4(position, 1.0)) + 0.5) + vec4(vec3(float(shiftX), float(shiftY), float(shiftZ)) * (giStep * 0.25), 1.0);
		giSample = (textureLod(lightVoxelTexture, giSpace.xyz, 0.5).rgb + (textureLod(lightVoxelTexture, giSpace.xyz, 1.5).rgb * 3.0)) / 2.0;

		giTempSample = textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 1.0), 0.5).rgb * 1.0;
		giNormal = textureLod(lightNormalTexture, giSpace.xyz + (normal * giStep * 1.0), 0.5).rgb * 1.0;
		giSample += giTempSample * combineIntensity(abs(normal - giNormal)) * 1.0;
		giOcclusion = combineIntensity(clamp((textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 1.0), 0.5 * voxelRatio).rgb * 1.0 * voxelRatio), 0.0, 100.0));

		giTempSample = (textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 3.0), 1.5).rgb * 3.0) * (1.0 - giOcclusion);
		giNormal = (textureLod(lightNormalTexture, giSpace.xyz + (normal * giStep * 3.0), 1.5).rgb * 3.0);
		giSample += giTempSample * combineIntensity(abs(normal - giNormal)) * 1.0;
		giOcclusion += combineIntensity(clamp((textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 3.0), 1.5 * voxelRatio).rgb * 3.0 * voxelRatio), 0.0, 100.0));

		giTempSample = (textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 7.0), 2.5).rgb * 5.0) * (1.0 - giOcclusion);
		giNormal = (textureLod(lightNormalTexture, giSpace.xyz + (normal * giStep * 7.0), 2.5).rgb * 5.0);
		giSample += giTempSample * combineIntensity(abs(normal - giNormal)) * 1.0;
		giOcclusion += combineIntensity(clamp((textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 7.0), 2.5 * voxelRatio).rgb * 5.0 * voxelRatio), 0.0, 100.0));

		giTempSample = (textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 15.0), 3.5).rgb * 7.0) * (1.0 - giOcclusion);
		giNormal = (textureLod(lightNormalTexture, giSpace.xyz + (normal * giStep * 15.0), 3.5).rgb * 7.0);
		giSample += giTempSample * combineIntensity(abs(normal - giNormal)) * 1.0;
		//giOcclusion += combineIntensity(clamp((textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 15.0), 3.5).rgb * 7.0), 0.0, 1.0));
		//giSample += (textureLod(lightVoxelTexture, giSpace.xyz + (normal * giStep * 31.0), 4.5).rgb * 9.0) - giOcclusion;

		giSample = clamp(giSample, 0.0, 1000.0);
	}

	intensity = intensity + (giSample * 2.0);

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

	fragment_color = vec4(sample_color.rgb * intensity, sample_color.a);

	//voxelSample = textureLod(voxelOcclusionTexture, voxelSpace.xyz, 0).rgb;
	//sample_color = vec4(0.0, 0.0, 0.0, sample_color.a);
	//sample_color = sample_color + vec4(voxelSample.rgb, sample_color.a);
	//fragment_color = vec4(sample_color.rgb, sample_color.a);
}