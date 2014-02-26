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

//const float kernelWeights[25] = { 
//	0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031,
//	0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
//	0.01093176, 0.11391157, 0.24880573, 0.11391157, 0.01093176,
//	0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
//	0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031
//};

const float kernelWeights[49] = { 
0.010441772214913722, 0.014272210436438351, 0.0172155719545337, 0.018325880952702808, 0.0172155719545337, 0.014272210436438351, 0.010441772214913722, 
0.014272210436438351, 0.01950779872894046, 0.023530896926464068, 0.025048509391588265, 0.023530896926464068, 0.01950779872894046, 0.014272210436438351, 
0.0172155719545337, 0.023530896926464068, 0.028383679668708053, 0.030214269731039196, 0.028383679668708053, 0.023530896926464068, 0.0172155719545337, 
0.018325880952702808, 0.025048509391588265, 0.030214269731039196, 0.03216292270894079, 0.030214269731039196, 0.025048509391588265, 0.018325880952702808, 
0.0172155719545337, 0.023530896926464068, 0.028383679668708053, 0.030214269731039196, 0.028383679668708053, 0.023530896926464068, 0.0172155719545337, 
0.014272210436438351, 0.01950779872894046, 0.023530896926464068, 0.025048509391588265, 0.023530896926464068, 0.01950779872894046, 0.014272210436438351, 
0.010441772214913722, 0.014272210436438351, 0.0172155719545337, 0.018325880952702808, 0.0172155719545337, 0.014272210436438351, 0.010441772214913722
};

const float errorCorrectWeights = 1.01238;
//vec2 lod;

vec4 calculateCoverage(in ivec2 coords)
{
	vec4 sample_color = vec4(0.0);
	for(int y = 0; y < coverageLayerPrecision; y++)
	{
		for(int x = 0; x < coverageLayerPrecision; x++)
		{
			sample_color += texelFetch(framebufferTexture, coords + ivec2(x, y), 0);
		}
	}
	return sample_color / (coverageLayerPrecision * coverageLayerPrecision);
}

void main()
{
	coords = ivec2(floor(((position.xy / 2.0) + 0.5) * float(coverageLayerPrecision) * float(coverageLayerResolution))) - 1;
	sample_color = vec4(0.0);
	for(int y = -3; y < 4; y++)
	{
		for(int x = -3; x < 4; x++)
		{
			sample_color += calculateCoverage(coords + clamp(ivec2(x, y), 0, coverageLayerResolution)) * kernelWeights[((y + 2) * 5) + (x + 2)] * errorCorrectWeights;
		}
	}

	//sample_color = texture(framebufferTexture, (position.xy / coverageLayerPrecision) + 0.5);
	fragment_color = vec4(sample_color.rgb, 1.0);
	//fragment_color = vec4(1.0);
}