#version 430
//Change extension to glsl for nsight debugging
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;
layout(location = 3) in vec3 vertex_tangent;
layout(location = 4) in vec3 vertex_bitangent;

uniform mat4 mvp;

out vec3 normal;
out vec2 uv;

void main()
{
	//color = vertex_normal;
	normal = vertex_normal;
	uv = vertex_uv;
	gl_Position = mvp * vec4(vertex_position, 1.0);
}