#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec3 OutNormalW;
layout (location = 2) out vec3 OutEyePosW;
layout (location = 3) out vec3 OutColor;
layout (location = 4) out vec2 OutTex;

layout (std140, set = 0, binding = 0) uniform UBO_input
{
   // Camera
   mat4 projection;
   mat4 view;
   vec3 eyePos;
   float pad;
} per_frame_vs;

layout(push_constant) uniform PushConsts {
   mat4 world;
   vec4 color;
} pushConsts;

void main(void)
{
   OutPosW = (pushConsts.world * vec4(InPosL.xyz, 1.0)).xyz;
   OutColor = pushConsts.color.rgb;
   OutNormalW = InNormalL.xyz;
   OutEyePosW = per_frame_vs.eyePos;
   OutTex = InTex;

   gl_Position = per_frame_vs.projection * per_frame_vs.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
}
