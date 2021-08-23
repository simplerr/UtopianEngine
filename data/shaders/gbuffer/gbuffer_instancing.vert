#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"
#include "shared_variables.glsl"

// Instancing input
layout (location = 6) in mat4 InInstanceWorld;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec3 OutPosW;
layout (location = 2) out vec3 OutNormalW;
layout (location = 3) out vec2 OutTex;
layout (location = 4) out vec3 OutNormalV;
layout (location = 5) out vec2 OutTextureTiling;
layout (location = 6) out mat3 OutTBN;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main()
{
   vec3 T = normalize(mat3(InInstanceWorld) * InTangentL.xyz);
   vec3 B = normalize(mat3(InInstanceWorld) * InBitangentL);
   vec3 N = normalize(mat3(InInstanceWorld) * InNormalL);
   OutTBN = mat3(T, B, N); // = transpose(mat3(T, B, N));

   OutColor = vec4(1.0);
   OutPosW = (InInstanceWorld * vec4(InPosL.xyz, 1.0)).xyz;
   OutNormalW = transpose(inverse(mat3(InInstanceWorld))) * InNormalL;
   mat3 normalMatrix = transpose(inverse(mat3(sharedVariables.viewMatrix * InInstanceWorld)));
   OutNormalV = normalMatrix * InNormalL;
   OutTex = InTex;
   OutTextureTiling = vec2(1.0, 1.0);

   gl_Position = sharedVariables.projectionMatrix * sharedVariables.viewMatrix * InInstanceWorld * vec4(InPosL.xyz, 1.0);
}