#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"
#include "shared_variables.glsl"

layout (set = 0, binding = 8) uniform UBO_constants
{
   mat4 projection;
   mat4 world;
} ubo_constants;

layout(push_constant) uniform PushConsts {
   mat4 view;
} pushConsts;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec3 OutPosL;
layout (location = 2) out vec2 OutTex;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main()
{
   OutPosW = (ubo_constants.world * vec4(InPosL.xyz, 1.0)).xyz;
   OutPosL = InPosL.xyz;
   OutTex = InTex;

   // Removes the translation components of the matrix to always keep the skybox at the same distance
   mat4 viewNoTranslation = mat4(mat3(pushConsts.view));
   gl_Position = ubo_constants.projection * viewNoTranslation * ubo_constants.world * vec4(InPosL.xyz, 1.0);
}
