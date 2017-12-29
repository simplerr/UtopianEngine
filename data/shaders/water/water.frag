#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTexCoord;
layout (location = 1) in vec4 InClipSpace;
layout (location = 2) in float InMoveFactor;

layout (set = 0, binding = 1) uniform sampler2D reflectionTexture;
layout (set = 0, binding = 2) uniform sampler2D refractionTexture;
layout (set = 0, binding = 3) uniform sampler2D dudvTexture;

layout (location = 0) out vec4 outFragColor;

const float waveStrength = 0.02;

void main(void)
{
  // Transform  from homogeneous clip space to texture coordinate
  vec2 ndc = (InClipSpace.xy/InClipSpace.w);
  vec2 uv = ndc / 2 + 0.5f;

  vec2 distortion1 = (texture(dudvTexture, vec2(InTexCoord.x + InMoveFactor, InTexCoord.y)).rg * 2.0f - 1.0f) * waveStrength;
  vec2 distortion2 = (texture(dudvTexture, vec2(-InTexCoord.x + InMoveFactor, InTexCoord.y + InMoveFactor)).rg * 2.0f - 1.0f) * waveStrength;
  vec2 totalDistortion = distortion1 + distortion2;

  vec2 reflectTexCoords = vec2(uv.x, -uv.y);
  vec2 refractTexCoords = vec2(uv.x, uv.y);

  reflectTexCoords += totalDistortion;
  reflectTexCoords.x = clamp(reflectTexCoords.x, 0.001f, 0.999f);
  reflectTexCoords.y = clamp(reflectTexCoords.y, -0.999, -0.001);

  refractTexCoords += totalDistortion;
  refractTexCoords = clamp(refractTexCoords, 0.001f, 0.999f);

  vec4 reflectColor = texture(reflectionTexture, reflectTexCoords);
  vec4 refractColor = texture(refractionTexture, refractTexCoords);

  outFragColor = mix(reflectColor, refractColor, 0.5);
  outFragColor.b += 0.5f;

  //outFragColor = texture(dudvTexture, vec2(InTexCoord.x, InTexCoord.y));
}
