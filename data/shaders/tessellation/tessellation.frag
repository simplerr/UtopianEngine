#version 450

layout (location = 0) in vec3 InNormalL;
layout (location = 1) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

void main() 
{
    OutColor = vec4(InTex.x, InTex.y, 0, 1);
}