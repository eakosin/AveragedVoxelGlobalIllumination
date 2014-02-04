#version 430
//Change extension to glsl for nsight debugging
in vec3 normal;
in vec2 uv;
out vec4 fragment_color;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D opacityTexture;

vec4 sample_color;
vec3 intensity;

vec3 lightposition;

void main()
{
	lightposition = vec3(0.0, 0.75, 0.5);
	intensity = ((normal + 1) / 2) * lightposition;
	intensity = vec3(intensity.x + intensity.y + intensity.z);
	sample_color = texture(diffuseTexture, uv);
	fragment_color = vec4(sample_color.rgb * intensity, sample_color.a);
	//if(fragment_color.a < 0.5)
	//{
	//	discard;
	//}
}