#version 450

layout (location = 0) in vec3 InColor;

layout (location = 1) out vec4 OutColor;

void main() 
{
	OutColor = vec4(InColor, 1.0f);
	OutColor = vec4(1, 0, 0, 1.0f);
}