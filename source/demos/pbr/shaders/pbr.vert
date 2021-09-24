#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InNormalL;
layout (location = 2) in vec2 InTex;
layout (location = 3) in vec3 InColor;
layout (location = 4) in vec4 InTangentL;
layout (location = 5) in vec4 InIndices;
layout (location = 6) in vec4 InWeights;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec3 OutNormalW;
layout (location = 2) out vec3 OutEyePosW;
layout (location = 3) out vec3 OutColor;
layout (location = 4) out vec2 OutTex;
layout (location = 5) out vec4 OutTangentL;
layout (location = 6) out int OutDebugChannel;

layout (std140, set = 0, binding = 0) uniform UBO_input
{
   // Camera
   mat4 projection;
   mat4 view;
   vec3 eyePos;
   int debugChannel;
} per_frame_vs;

layout(push_constant) uniform PushConsts {
   mat4 world;
} pushConsts;

void main(void)
{
   OutPosW = (pushConsts.world * vec4(InPosL.xyz, 1.0)).xyz;
   OutColor = vec3(1.0f);
   OutNormalW = mat3(pushConsts.world) * InNormalL.xyz;
   OutEyePosW = per_frame_vs.eyePos;
   OutTex = InTex;
   OutTangentL = InTangentL;
   OutDebugChannel = per_frame_vs.debugChannel;

   gl_Position = per_frame_vs.projection * per_frame_vs.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
}
