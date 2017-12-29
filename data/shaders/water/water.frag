#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 InClipSpace;

layout (set = 0, binding = 1) uniform sampler2D reflectionTexture;
layout (set = 0, binding = 2) uniform sampler2D refractionTexture;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
  vec2 ndc = (InClipSpace.xy/InClipSpace.w) / 2 + 0.5f;

  vec2 reflectTexCoords = vec2(ndc.x, -ndc.y);
  vec2 refractTexCoords = vec2(ndc.x, ndc.y);

  vec4 reflectColor = texture(reflectionTexture, reflectTexCoords);
  vec4 refractColor = texture(refractionTexture, refractTexCoords);

  outFragColor = mix(reflectColor, refractColor, 0.5);
}
