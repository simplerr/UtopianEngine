#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, set = 0, binding = 0) uniform UBO_settings
{
	int algorithm; // 0 = Reinhard
    float exposure;
} settings_ubo;

layout (set = 0, binding = 1) uniform sampler2D hdrSampler;
layout (set = 0, binding = 2) uniform sampler2D bloomSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

void main() 
{
    int t = settings_ubo.algorithm;

    const float gamma = 1.0;
    vec3 hdrColor = texture(hdrSampler, InTex).rgb;
    vec3 bloomColor = texture(bloomSampler, InTex).rgb;

    hdrColor += bloomColor;
  
    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * settings_ubo.exposure);

    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    OutColor = vec4(mapped, 1.0);
}