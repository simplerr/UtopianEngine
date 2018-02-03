#version 450

layout (location = 0) in vec3 InColor;

layout (location = 0) out vec4 OutColor;

void main() 
{
	OutColor = vec4(InColor, 1.0f);
	OutColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}