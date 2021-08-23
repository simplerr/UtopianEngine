#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"

layout (location = 0) out vec3 outNormal;

void main(void)
{
   outNormal = InNormalL;
   gl_Position = vec4(InPosL.xyz, 1.0);
}