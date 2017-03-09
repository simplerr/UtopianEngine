#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
	outFragColor = vec4(inColor, 1.0);
	outFragColor = vec4(inNormal.r, inNormal.g, inNormal.b, 1.0);
	outFragColor = vec4(0, 0, inNormal.b, 1.0);
}