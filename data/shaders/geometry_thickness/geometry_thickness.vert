#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"
#include "shared_variables.glsl"

layout (push_constant) uniform PushConstants {
   mat4 world;
   vec4 color;
} pushConstants;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec2 OutTex;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main()
{
   OutPosW = (pushConstants.world * vec4(InPosL.xyz, 1.0)).xyz;
   OutTex = InTex;

   gl_Position = sharedVariables.projectionMatrix * sharedVariables.viewMatrix * pushConstants.world * vec4(InPosL.xyz, 1.0);
}