#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "noise.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out float OutHeight;

void main()
{
   float height = fbm(InTex * 4);
   height = height * 2 - 1; // [0, 1] -> [-1, 1]
   OutHeight = height;
}