#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"
#include "shared_variables.glsl"

layout(std430, set = 1, binding = 0) readonly buffer JointMatrices {
   mat4 jointMatrices[];
};

layout (push_constant) uniform PushConstants {
   mat4 world;
   vec4 color;
} pushConstants;

layout (location = 0) out vec4 OutColor;

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

   OutColor = pushConstants.color;

   gl_Position = sharedVariables.projectionMatrix * sharedVariables.viewMatrix * pushConstants.world * skinMat * vec4(InPosL.xyz, 1.0);
}