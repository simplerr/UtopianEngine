#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 2) uniform sampler2D texSampler;

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;
layout (location = 4) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

void main() 
{
	OutFragColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}