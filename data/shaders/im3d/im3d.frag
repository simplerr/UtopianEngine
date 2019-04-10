#version 450

layout (location = 0) in float InSize;
layout (location = 1) in vec4 InColor;

layout (location = 0) out vec4 OutColor;

void main() 
{
    OutColor = InColor;
    float tmp = InSize;
}