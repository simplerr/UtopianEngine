#version 450

layout (location = 0) in vec3 InColor;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

void main() 
{
	outPosition = vec4(1, 0, 0, 1.0f);
	outNormal = vec4(1, 1, 1, 1.0f);
	outAlbedo = vec4(1, 0, 1, 1.0f);
}