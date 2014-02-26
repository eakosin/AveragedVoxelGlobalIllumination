#version 430
//Change extension to glsl for nsight debugging
layout(location = 0) in vec3 vertex_position;

out vec3 position;

void main()
{
	//color = vertex_normal;
	position = vertex_position;
	gl_Position = vec4(vertex_position, 1.0);
}