#version 450

layout (location = 0) in vec4 InColor;
layout (location = 1) in vec2 InTex;
layout (location = 2) in float InEyeDist;

layout (set = 1, binding = 0) uniform sampler2D textureSampler;
layout (set = 2, binding = 0) uniform sampler2D textureSampler2;

layout (location = 0) out vec4 OutColor;

void main() 
{
	vec4 color = texture(textureSampler, InTex);
	vec4 color2 = texture(textureSampler2, InTex);

	// Discard fragment if it's outside grass view distance
	if (InEyeDist > 2000.0f)
	 	discard;

	if (color.a < 0.1f)
	 	discard;

	color.xyz *= InColor.xyz;
	OutColor = color * InColor.a;
}