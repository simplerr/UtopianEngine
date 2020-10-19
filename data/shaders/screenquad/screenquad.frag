#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTex;

layout (set = 0, binding = 0) uniform sampler3D texSampler;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
  outFragColor = texture(texSampler, vec3(InTex.x, 81.0f / 256.0f, InTex.y));
}
