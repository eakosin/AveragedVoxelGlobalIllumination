#version 430
//Change extension to glsl for nsight debugging
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

uniform mat4 mvp;

out vec3 color;

void main()
{
	//color = vertex_normal;
	color = (vertex_normal / 2) + 0.5;
	gl_Position = mvp * vec4(vertex_position, 1.0);
}