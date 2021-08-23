#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"

// Instancing input
layout (location = 6) in mat4 InInstanceWorld;

layout (std140, set = 0, binding = 0) uniform UBO_cascadeTransforms 
{
   mat4 viewProjection[4];
} cascade_transforms;

layout (push_constant) uniform PushConstants {
   mat4 world; // Used by shadowmap.vert
   uint cascadeIndex;
} pushConstants;

layout (location = 0) out vec2 OutTex;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main()
{
   OutTex = InTex;

   gl_Position = cascade_transforms.viewProjection[pushConstants.cascadeIndex] * InInstanceWorld * vec4(InPosL.xyz, 1.0);
}