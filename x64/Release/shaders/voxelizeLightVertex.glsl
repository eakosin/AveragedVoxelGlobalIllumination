#version 430
//Change extension to glsl for nsight debugging
layout(location = 0) in vec3 vertex_position;

uniform mat4 mvp;

uniform int layer;
uniform uint lightResolution;

out vec3 position;
out vec3 normal;
out vec2 uv;
out float resolution;

void main()
{
	position = (mvp * vec4(vertex_position.xy, ((float(layer + 0.0) * 2.0) / float(lightResolution)) - 1.0, 1.0)).xyz;
	resolution = float(lightResolution);
	gl_Position = vec4(vertex_position, 1.0);
}