#version 430
//Change extension to glsl for nsight debugging
in vec3 normal;
in vec2 uv;
out vec4 fragment_colour;

uniform sampler2D diffuseTexture;

void main()
{
	fragment_colour = texture(diffuseTexture, -uv);
}