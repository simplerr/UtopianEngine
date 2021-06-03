#version 450

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;

layout (location = 0) out vec4 OutColor;

void main()
{
   vec3 lightDir = vec3(0.5, -0.5, -0.5);
   float diffuseFactor = dot(lightDir, InNormalW) + 0.3;
   OutColor = vec4(vec3(InColor.rgb * diffuseFactor), 1.0f);
}