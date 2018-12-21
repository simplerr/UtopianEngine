#version 450

layout (location = 0) in vec3 InColor;
layout (location = 1) in vec2 InTex;

layout (set = 1, binding = 0) uniform sampler2D textureSampler;

layout (location = 0) out vec4 OutColor;

void main() 
{
	vec4 color = texture(textureSampler, InTex);

	if (color.a < 0.1f)
		discard;

	OutColor = color;
}