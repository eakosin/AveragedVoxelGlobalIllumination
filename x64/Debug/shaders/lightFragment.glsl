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
layout(early_fragment_tests) in;
in vec3 position;
in vec3 normal;
in vec2 uv;

layout (location = 0) out vec4 fragment_color;
layout (location = 1) out vec4 fragment_normal;

uniform sampler2D diffuseTexture;

vec4 sample_color;

// Render scene from light perspective
void main()
{

	sample_color = texture(diffuseTexture, uv);

	if(sample_color.a < 0.5)
	{
		discard;
	}

	fragment_normal = vec4(((normal + 1.0) / 2.0), sample_color.a);

	fragment_color = vec4(sample_color.rgb, sample_color.a);
}