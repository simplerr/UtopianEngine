#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"

layout(push_constant) uniform PushConsts {
   mat4 mvp;
   float roughness;
} pushConsts;

layout (location = 0) out vec3 OutUVW;
layout (location = 1) out float OutRoughness;

out gl_PerVertex {
   vec4 gl_Position;
};

void main()
{
   OutUVW = InPosL;
   OutRoughness = pushConsts.roughness;

   gl_Position = pushConsts.mvp * vec4(InPosL.xyz, 1.0);
}
