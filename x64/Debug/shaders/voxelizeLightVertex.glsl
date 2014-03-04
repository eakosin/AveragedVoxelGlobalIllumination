#version 430
//Change extension to glsl for nsight debugging
layout(location = 0) in vec3 vertex_position;

uniform mat4 mvp;

uniform int layer;

out vec3 position;
out vec3 normal;
out vec2 uv;

void main()
{
	position = (mvp * vec4(vertex_position.xy, ((float(layer + 1.0) * 2.0) / 128) - 1.0, 1.0)).xyz;
	gl_Position = vec4(vertex_position, 1.0);
}