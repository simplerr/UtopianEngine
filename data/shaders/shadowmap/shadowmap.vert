#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"

layout (std140, set = 0, binding = 0) uniform UBO_cascadeTransforms
{
   mat4 viewProjection[4];
} cascade_transforms;

layout (push_constant) uniform PushConstants {
   mat4 world;
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

   gl_Position = cascade_transforms.viewProjection[pushConstants.cascadeIndex] * pushConstants.world * vec4(InPosL.xyz, 1.0);
}