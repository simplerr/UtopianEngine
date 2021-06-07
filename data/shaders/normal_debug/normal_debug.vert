#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (location = 0) out vec3 outNormal;

void main(void)
{
   outNormal = InNormalL;
   gl_Position = vec4(InPosL.xyz, 1.0);
}