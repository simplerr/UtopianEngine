#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (set = 0, binding = 0) uniform UBO_input
{
   mat4 world;
} input_ubo;

layout (location = 0) out vec3 outUVW;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main()
{
   outUVW = InPosL;

   // Removes the translation components of the matrix to always keep the skybox at the same distance
   mat4 viewNoTranslation = mat4(mat3(sharedVariables.viewMatrix));
   gl_Position = sharedVariables.projectionMatrix * viewNoTranslation * input_ubo.world * vec4(InPosL.xyz, 1.0);
}
