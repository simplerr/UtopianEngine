#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, set = 0, binding = 0) uniform UBO_settings
{
	float threshold;
} settings_ubo;

layout (set = 0, binding = 1) uniform sampler2D hdrSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

float rgb2luma(vec3 rgb)
{
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

void main() 
{
    vec3 hdrColor = texture(hdrSampler, InTex).rgb;
  
    if (rgb2luma(hdrColor) > settings_ubo.threshold)
        OutColor = vec4(hdrColor, 1.0f);
    else
        OutColor = vec4(vec3(0.0f), 1.0f);
}