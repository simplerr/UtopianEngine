#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTex;

layout (set = 0, binding = 1) uniform sampler2D reflectionTexture;
layout (set = 0, binding = 2) uniform sampler2D refractionTexture;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
  vec2 uv = InTex;
  //uv.x *= -1;
  outFragColor = texture(reflectionTexture, uv);
  outFragColor = texture(refractionTexture, uv);
  //outFragColor = vec4(0, 0, 1, 1);
}
