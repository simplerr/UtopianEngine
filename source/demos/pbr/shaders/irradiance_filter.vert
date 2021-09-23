#version 450

#extension GL_GOOGLE_include_directive : enable

#include "vertex.glsl"

layout(push_constant) uniform PushConsts {
   mat4 mvp;
} pushConsts;

layout (location = 0) out vec3 OutUVW;

out gl_PerVertex {
   vec4 gl_Position;
};

void main()
{
   OutUVW = InPosL;
   gl_Position = pushConsts.mvp * vec4(InPosL.xyz, 1.0);
}
