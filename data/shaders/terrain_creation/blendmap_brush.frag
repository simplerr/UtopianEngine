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
    int mode; // 0 = height, 1 = blend
    int operation; // 0 = add, 1 = remove
    int blendLayer; // 0 = grass, 1 = rocks, 2 = dirt
} ubo_brush;

void main() 
{
    vec3 blend = vec3(1, 0, 0);
    vec3 layer = vec3(1, 0, 0);

    if (ubo_brush.blendLayer == 1)
        layer = vec3(0, 1, 0);
    else if (ubo_brush.blendLayer == 2)
        layer = vec3(0, 0, 1);

    vec2 center = ubo_brush.pos;
    if (distance(center, InTex) < ubo_brush.radius)
        blend = layer;
    else
        discard;

    OutFragColor = vec4(blend, 1.0);
}