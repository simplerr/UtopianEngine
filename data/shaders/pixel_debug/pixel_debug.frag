#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, set = 0, binding = 0) uniform UBO_input
{
 	vec2 mousePosUV;
} ubo_input;

layout (set = 0, binding = 1) uniform sampler2D debugSampler;

layout (location = 0) in vec2 InTex;

// This buffer is read from in PixelDebugJob
layout(std430, set = 0, binding = 2) buffer UBO_output 
{
	vec4 data;
} output_SSBO;

void main() 
{
    vec4 debugData = texture(debugSampler, ubo_input.mousePosUV);

    output_SSBO.data = debugData;
}