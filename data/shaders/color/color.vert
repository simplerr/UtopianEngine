#version 450

#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (push_constant) uniform PushConstants {
   mat4 world;
   vec4 color;

   // These exceeds the 128 byte limit
   // vec2 textureTiling;
   // vec2 pad;
} pushConstants;

layout (location = 0) out vec4 OutColor;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main()
{
   OutColor = pushConstants.color;

   gl_Position = sharedVariables.projectionMatrix * sharedVariables.viewMatrix * pushConstants.world * vec4(InPosL.xyz, 1.0);
}