#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

void main() 
{
    vec3 color = mix(vec3(1,0,0), vec3(0, 1, 0), vec3(InTex.x));
    color = mix(color, vec3(0, 0, 1), vec3(InTex.y));

    OutColor = vec4(color, 1);
}