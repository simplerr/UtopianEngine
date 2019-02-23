#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 InTex;
layout (location = 0) out vec4 OutFragColor;

layout (std140, set = 0, binding = 0) uniform UBO_brush 
{
    vec2 pos;
} ubo_brush;

void main() 
{
    vec2 center = ubo_brush.pos;
    //center = vec2(0.5, 0.5);
    float height = smoothstep(0.1, 0.0, distance(center, InTex));

    OutFragColor = vec4(-height);
}