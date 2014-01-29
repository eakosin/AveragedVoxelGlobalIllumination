#version 430
//Change extension to glsl for nsight debugging
in vec3 color;
out vec4 frag_colour;

void main()
{
	frag_colour = vec4(color, 1.0);
}