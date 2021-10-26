#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"
#include "shared_variables.glsl"

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec3 OutPosW;
layout (location = 2) out vec3 OutNormalW;
layout (location = 3) out vec2 OutTex;
layout (location = 4) out vec3 OutNormalV;
layout (location = 5) out vec2 OutTextureTiling;
layout (location = 6) out vec3 OutTangentL;
layout (location = 7) out mat3 OutTBN;

layout(std430, set = 2, binding = 0) readonly buffer JointMatrices {
   mat4 jointMatrices[];
};

layout (push_constant) uniform PushConstants {
   mat4 world;
   vec4 color;
   vec2 textureTiling;
   vec2 pad;
} pushConstants;

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

   vec3 bitangentL = cross(InNormalL, InTangentL.xyz);
   vec3 T = normalize(mat3(pushConstants.world) * InTangentL.xyz);
   vec3 B = normalize(mat3(pushConstants.world) * bitangentL);
   vec3 N = normalize(mat3(pushConstants.world) * InNormalL);
   OutTBN = mat3(T, B, N); // = transpose(mat3(T, B, N));

   OutColor = pushConstants.color;
   OutPosW = (pushConstants.world * vec4(InPosL.xyz, 1.0)).xyz;
   OutNormalW = mat3(transpose(inverse(pushConstants.world))) * InNormalL;
   mat3 normalMatrix = transpose(inverse(mat3(sharedVariables.viewMatrix * pushConstants.world)));
   OutNormalV = normalMatrix * InNormalL;
   OutTex = InTex;
   OutTextureTiling = pushConstants.textureTiling;
   OutTangentL = InTangentL.xyz;

   gl_Position = sharedVariables.projectionMatrix * sharedVariables.viewMatrix * pushConstants.world * skinMat * vec4(InPosL.xyz, 1.0);
}