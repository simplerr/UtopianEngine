#version 450

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;
layout (location = 4) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;

void main()
{
   vec4 diffuse = texture(diffuseSampler, InTex);

   vec3 lightDir = vec3(0.5, -0.5, -0.5);
   float diffuseFactor = max(dot(lightDir, InNormalW), 0.0) + 0.2;
   OutColor = vec4(diffuse.rgb * diffuseFactor, 1.0f);
}
