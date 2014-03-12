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
layout(location = 0) in vec3 vertex_position;

uniform mat4 mvp;

uniform int layer;
uniform uint lightResolution;

out vec3 position;
out vec3 normal;
out vec2 uv;
out float resolution;

void main()
{
	// Scale vertex position into the same perspective as the rendered light's perspective
	position = (mvp * vec4(vertex_position.xy, ((float(layer + 0) * 2.0) / float(lightResolution)) - 1.0, 1.0)).xyz;
	resolution = float(lightResolution);
	gl_Position = vec4(vertex_position, 1.0);
}