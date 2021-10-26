#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"

layout (std140, set = 0, binding = 0) uniform UBO_cascadeTransforms
{
   mat4 viewProjection[4];
} cascade_transforms;

layout(std430, set = 2, binding = 0) readonly buffer JointMatrices {
   mat4 jointMatrices[];
};

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
   // Calculate skinned matrix from weights and joint indices of the current vertex
   mat4 skinMat = InJointWeights.x * jointMatrices[int(InJointIndices.x)] +
                  InJointWeights.y * jointMatrices[int(InJointIndices.y)] +
                  InJointWeights.z * jointMatrices[int(InJointIndices.z)] +
                  InJointWeights.w * jointMatrices[int(InJointIndices.w)];

   OutTex = InTex;

   gl_Position = cascade_transforms.viewProjection[pushConstants.cascadeIndex] * pushConstants.world * skinMat * vec4(InPosL.xyz, 1.0);
}