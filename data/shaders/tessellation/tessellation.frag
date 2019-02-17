#version 450

layout (set = 0, binding = 3) uniform sampler2D samplerNormalmap;

layout (location = 0) in vec3 InNormalL;
layout (location = 1) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

void main() 
{
    //OutColor = vec4(InTex.x, InTex.y, 0, 1);
    vec3 normal = texture(samplerNormalmap, InTex).xyz;
    OutColor = vec4(normal, 1.0);
}