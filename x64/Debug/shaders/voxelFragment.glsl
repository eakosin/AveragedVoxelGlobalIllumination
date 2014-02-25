#version 430
//Change extension to glsl for nsight debugging
layout(early_fragment_tests) in;
out vec4 fragment_color;

void main()
{
	fragment_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	if(fragment_color.a < 0.5)
	{
		discard;
	}
}