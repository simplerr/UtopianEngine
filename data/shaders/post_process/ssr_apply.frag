#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform sampler2D reflectionSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

void main() 
{
    vec3 reflectionColor = texture(reflectionSampler, InTex).rgb;

    OutFragColor = vec4(reflectionColor / 2.0f, 1.0f);
}