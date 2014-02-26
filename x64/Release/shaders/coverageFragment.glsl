#version 430
//Change extension to glsl for nsight debugging
//layout(early_fragment_tests) in;
in vec3 position;
out vec4 fragment_color;

uniform sampler2D framebufferTexture;
uniform int coverageLayerResolution;
uniform int coverageLayerPrecision;

vec4 sample_color;

ivec2 coords;

//vec2 lod;

void main()
{
	coords = ivec2(floor(((position.xy / 2.0) + 0.5) * float(coverageLayerPrecision) * float(coverageLayerResolution))) - 1;
	sample_color = texelFetch(framebufferTexture, coords, 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(1, 0), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(2, 0), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(3, 0), 0);

	sample_color += texelFetch(framebufferTexture, coords + ivec2(0, 1), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(1, 1), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(2, 1), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(3, 1), 0);

	sample_color += texelFetch(framebufferTexture, coords + ivec2(0, 2), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(1, 2), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(2, 2), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(3, 2), 0);

	sample_color += texelFetch(framebufferTexture, coords + ivec2(0, 3), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(1, 3), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(2, 3), 0);
	sample_color += texelFetch(framebufferTexture, coords + ivec2(3, 3), 0);

	sample_color /= vec4(16.0);

	//sample_color = texture(framebufferTexture, (position.xy / coverageLayerPrecision) + 0.5);
	fragment_color = vec4(sample_color.rgb, 1.0);
	//fragment_color = vec4(1.0);
}