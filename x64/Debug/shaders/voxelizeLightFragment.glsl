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

void main()
{
	// Computer coordinate for texture based on position in space
	vec3 coordinate = vec3(((position.xy * 0.25) + 0.5), (position.z * 0.25) + 0.625);
	depth = texture(lightDepthFramebufferTexture, coordinate.xy).r;

	//if((abs(depth - position.z) * (resolution * 4.0)) > 1.0)
	//{
	//	discard;
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