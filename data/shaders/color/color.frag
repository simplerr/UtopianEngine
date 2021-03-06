#version 450

layout (location = 0) in vec4 InColor;

layout (location = 0) out vec4 OutColor;

void main() 
{
   // Alpha channel is brightness
   OutColor = vec4(vec3(InColor.rgb) * InColor.a, 1.0f);
}