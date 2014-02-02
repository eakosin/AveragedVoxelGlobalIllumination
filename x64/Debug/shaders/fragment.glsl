#version 430
//Change extension to glsl for nsight debugging
in vec3 normal;
in vec2 uv;
out vec4 fragment_colour;

uniform sampler2D diffuseTexture;
uniform sampler2D opacityTexture;

void main()
{
	fragment_colour = texture(diffuseTexture, uv);
	if(fragment_colour.a < 0.5)
	{
		discard;
	}
}