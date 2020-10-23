#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 InPosL;
layout (location = 1) in vec4 InNormal;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec3 OutNormalW;
layout (location = 2) out vec3 OutEyePosW;
layout (location = 3) out vec3 OutColor;

layout (std140, set = 0, binding = 0) uniform UBO
{
    // Camera
    mat4 projection;
    mat4 view;

    vec4 clippingPlane;
    vec3 eyePos;

    float t;
} per_frame_vs;

layout(push_constant) uniform PushConsts {
    mat4 world;
    vec4 color;
} pushConsts;

void main(void)
{
    OutPosW = (pushConsts.world * vec4(InPosL.xyz, 1.0)).xyz;
    OutColor = pushConsts.color.rgb;
    OutNormalW = InNormal.xyz;
    OutEyePosW = per_frame_vs.eyePos;

    gl_Position = per_frame_vs.projection * per_frame_vs.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
}