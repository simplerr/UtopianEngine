#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"
#include "shared_variables.glsl"

layout (set = 0, binding = 0) uniform UBO_input
{
   mat4 world;
} input_ubo;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec3 OutPosL;
layout (location = 2) out vec2 OutTex;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main()
{
   OutPosW = (input_ubo.world * vec4(InPosL.xyz, 1.0)).xyz;
   OutPosL = InPosL.xyz;
   OutTex = InTex;

   // Removes the translation components of the matrix to always keep the skybox at the same distance
   mat4 viewNoTranslation = mat4(mat3(sharedVariables.viewMatrix));
   gl_Position = sharedVariables.projectionMatrix * viewNoTranslation * input_ubo.world * vec4(InPosL.xyz, 1.0);
}
