#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "../common/sky_color.glsl"
#include "shared_variables.glsl"

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InPosL;
layout (location = 2) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;
layout (location = 1) out vec4 OutSun;

void main()
{
	SkyOutput skyOutput = GetSkyColor(normalize(InPosL), sharedVariables.eyePos.xyz, sharedVariables.time);

	OutFragColor = skyOutput.skyColor;
	OutSun = skyOutput.sunColor;
}