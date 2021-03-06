#version 450

layout (location = 0) in float InSize;
layout (location = 1) in vec4 InColor;

layout (location = 0) out vec4 OutColor;

#define kAntialiasing 3.0

void main()
{
   OutColor = InColor;

   float d = length(gl_PointCoord.xy - vec2(0.5));
   d = smoothstep(0.5, 0.5 - (kAntialiasing / InSize), d);
   OutColor.a *= d;
}