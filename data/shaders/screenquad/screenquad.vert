#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec2 InTex;

layout (location = 0) out vec2 OutTex;

layout(push_constant) uniform PushConsts {
   mat4 world;
   vec3 color;
} pushConsts;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main(void)
{
   OutTex = InTex;
   gl_Position = pushConsts.world * vec4(InPosL.xyz, 1.0);
}