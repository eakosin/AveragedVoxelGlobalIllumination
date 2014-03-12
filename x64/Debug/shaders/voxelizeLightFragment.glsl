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
in vec3 position;
in float resolution;

layout (location = 0) out vec4 fragment_color;
layout (location = 1) out vec4 fragment_normal;

uniform sampler2D lightFramebufferTexture;
uniform sampler2D lightNormalFramebufferTexture;
uniform sampler2D lightDepthFramebufferTexture;

float depth;
vec4 color;
vec4 normal;
vec3 coordinate;

//const float kernelWeights[25] = { 
//	0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031,
//	0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
//	0.01093176, 0.11391157, 0.24880573, 0.11391157, 0.01093176,
//	0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
//	0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031
//};
//
//const float errorCorrectWeights = 1.01238;

const float kernelWeights[9] = { 
0.07511360795411207, 0.12384140315297386, 0.07511360795411207, 
0.12384140315297386, 0.20417995557165622, 0.12384140315297386, 
0.07511360795411207, 0.12384140315297386, 0.07511360795411207
};

const float errorCorrectWeights = 1.0;

void main()
{
	// Computer coordinate for texture based on position in space
	coordinate = vec3(((position.xy * 0.25) + 0.5), (position.z * 0.25) + 0.625);
	depth = texture(lightDepthFramebufferTexture, coordinate.xy).r;
	//depth = 0.0;

	//for(int y = -1; y < 2; y++)
	//{
	//	for(int x = -1; x < 2; x++)
	//	{
	//		//sample_color += calculateCoverage(coords + clamp(ivec2(x, y), 0, coverageLayerResolution)) * (1.0 / (radius * radius));
	//		depth += texture(lightDepthFramebufferTexture, coordinate.xy + vec2(x * (1 / resolution), y * (1 / resolution))).r * kernelWeights[((y + 1) * 3) + (x + 1)] * errorCorrectWeights;
	//	}
	//}

	// Sample light texture
	color = texture(lightFramebufferTexture, coordinate.xy);
	normal = texture(lightNormalFramebufferTexture, coordinate.xy);

	// Attenuate texture value based on distance from depth texture to only place light in the correct voxels
	color = color * ( 1.0 - clamp( abs( ( ( depth ) - coordinate.z ) * (resolution * 4.0) ) , 0.0, 1.0 ) );
	normal = normal * ( 1.0 - clamp( abs( ( ( depth ) - coordinate.z ) * (resolution * 4.0) ) , 0.0, 1.0 ) );

	fragment_color = color;
	fragment_normal = normal;
}