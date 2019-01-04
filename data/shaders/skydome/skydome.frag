#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InPosL;
layout (location = 2) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 0, binding = 1) uniform UBO_parameters 
{
	float sphereRadius;
} ubo_parameters;

const vec3 baseColor = vec3(0.18,0.27,0.47);

void main() 
{
	vec3 normalizedPos = InPosL / ubo_parameters.sphereRadius;
	OutFragColor = vec4(normalizedPos.y + baseColor, 1.0f);
}