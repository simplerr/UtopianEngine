#version 450

layout (location = 0) in float InSize;
layout (location = 1) in vec4 InColor;
layout (location = 2) in float InEdgeDistance;

layout (location = 0) out vec4 OutColor;

#define kAntialiasing 3.0

void main()
{
   OutColor = InColor;

   float d = abs(InEdgeDistance) / InSize;
   d = smoothstep(1.0, 1.0 - (kAntialiasing / InSize), d);
   OutColor.a *= d;
}