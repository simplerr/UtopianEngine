#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable
#include "shared_variables.glsl"

layout (std140, set = 0, binding = 0) uniform UBO_settings
{
   float outlineWidth;
} settings_ubo;

layout (set = 0, binding = 1) uniform sampler2D maskSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

// Reference: https://gist.github.com/Hebali/6ebfc66106459aacee6a9fac029d0115
void make_kernel(inout vec4 n[9], sampler2D tex, vec2 coord)
{
   float w = settings_ubo.outlineWidth / sharedVariables.viewportSize.x;
   float h = settings_ubo.outlineWidth / sharedVariables.viewportSize.y;

   n[0] = texture(tex, coord + vec2(-w, -h));
   n[1] = texture(tex, coord + vec2(0.0, -h));
   n[2] = texture(tex, coord + vec2(w, -h));
   n[3] = texture(tex, coord + vec2(-w, 0.0));
   n[4] = texture(tex, coord);
   n[5] = texture(tex, coord + vec2(w, 0.0));
   n[6] = texture(tex, coord + vec2(-w, h));
   n[7] = texture(tex, coord + vec2(0.0, h));
   n[8] = texture(tex, coord + vec2(w, h));
}

void main(void)
{
   vec4 n[9];
   make_kernel( n, maskSampler, InTex);

   vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
   vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
   vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));

   const vec3 outlineColor = vec3(0.9f, 0.9f, 0.1f);
   OutColor = vec4(outlineColor, sobel.a);
}
