#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout (set = 0, binding = 0) uniform sampler2D lightSampler;
layout (set = 0, binding = 1) uniform sampler2D depthSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

void main()
{
   vec4 color = texture(lightSampler, InTex);
   float depth = texture(depthSampler, InTex).r;

   OutFragColor = color;
   gl_FragDepth = depth;
}