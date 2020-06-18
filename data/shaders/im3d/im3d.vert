#version 450

#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"

layout (location = 0) in vec4 InPositionSize;
layout (location = 1) in vec4 InColor;

layout (location = 0) out float OutSize;
layout (location = 1) out vec4 OutColor;

void main()
{
    OutSize = InPositionSize.w;
	OutColor = InColor.abgr; // Swizzle for correct endianess

	vec3 pos = -InPositionSize.xyz; // Todo: Note the -1
	gl_Position = sharedVariables.projectionMatrix * sharedVariables.viewMatrix * vec4(pos, 1.0);
	gl_PointSize = InPositionSize.w;
}