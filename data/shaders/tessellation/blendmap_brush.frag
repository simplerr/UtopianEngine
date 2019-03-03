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
} ubo_brush;

void main() 
{
    vec3 color = vec3(1, 0, 0);

    vec2 center = ubo_brush.pos;
    //center = vec2(0.5, 0.5);
    //if (distance(ubo_brush.pos, InTex) < 0.1)
    //if (distance(center, InTex) < 0.01)
    if (distance(center, InTex) < ubo_brush.radius)
        color = vec3(0, 0, 1);
    else
        discard;

    OutFragColor = vec4(color, 1.0);
}