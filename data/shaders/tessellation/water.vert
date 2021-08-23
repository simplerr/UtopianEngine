#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"
#include "shared_water.glsl"
#include "shared_variables.glsl"

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
   gl_Position.xyz = calculateWavePosition(gl_Position.xz, sharedVariables.time, OutNormalL);
}