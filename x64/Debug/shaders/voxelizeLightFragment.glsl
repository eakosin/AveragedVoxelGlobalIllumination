#version 430
//Change extension to glsl for nsight debugging
in vec3 position;

layout (location = 0) out vec4 fragment_color;
layout (location = 1) out vec4 fragment_normal;

uniform sampler2D lightFramebufferTexture;
uniform sampler2D lightNormalFramebufferTexture;
uniform sampler2D lightDepthFramebufferTexture;

float depth;
vec4 color;
vec4 normal;

//vec2 lod;

void main()
{
	//vec3 coordinate = position;
	vec3 coordinate = vec3(((position.xy * 0.25) + 0.5), position.z);
	depth = texture(lightDepthFramebufferTexture, coordinate.xy).r;

	//if(abs(depth - position.z) > 1.0)
	//{
	//	discard;
	//}

	color = texture(lightFramebufferTexture, coordinate.xy);
	normal = texture(lightNormalFramebufferTexture, coordinate.xy);

	//color = vec4(coordinate, 1.0);

	color = color * ( 1.0 - clamp( abs( ( ( depth ) - coordinate.z ) * 16.0 ) , 0.0, 1.0 ) );

	//color = vec4(vec3(1.0 - abs(depth - position.z)), 1.0);

	fragment_color = color;
	fragment_normal = normal;

}