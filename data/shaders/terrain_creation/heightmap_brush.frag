#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 InTex;
layout (location = 0) out vec4 OutFragColor;

layout (std140, set = 0, binding = 0) uniform UBO_brush 
{
    vec2 pos;
    float radius;
    float strength;
    int mode; // 0 = height, 1 = blend, 2 = vegetation, 3 = height_flat
    int operation; // 0 = add, 1 = remove
} ubo_brush;

void main() 
{
    vec2 center = ubo_brush.pos;

    float height = ubo_brush.radius - distance(center, InTex);
    height = clamp(height, 0.0f, 1.0f);

    // Flat shape
    if (ubo_brush.mode == 3)
        height = pow(height, 0.2) * 0.02;

    height *= ubo_brush.strength;

    if (ubo_brush.operation == 0)
        height *= -1;

    OutFragColor = vec4(height);
}