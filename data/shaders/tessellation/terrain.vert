#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared.glsl"

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (location = 0) out vec3 OutNormalL;
layout (location = 1) out vec2 OutTex;

void main()
{
	OutNormalL = InNormalL;
    OutTex = InTex;
    gl_Position = vec4(InPosL.xyz, 1.0);

    // Need to displace the Y coordinate here so that the tessellation factor
    // calculation in the .tesc shader works as expected. Otherwise all vertices will
    // have y=0.0.
    gl_Position.y = getHeight(OutTex);

	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InColor;
	vec3 temp2 = InTangentL;
	temp2 = InBitangentL;
}