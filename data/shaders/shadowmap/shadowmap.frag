#version 450
#extension GL_GOOGLE_include_directive : enable

#include "material.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out float OutColor;

const float NEAR_PLANE = 1.0f; //todo: specialization const
const float FAR_PLANE = 10000.0f; //todo: specialization const 

float linearDepth(float depth)
{
   float z = depth * 2.0f - 1.0f; 
   return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));  
}

void main()
{
   vec4 color = texture(diffuseSampler, InTex);

   // Todo: Remove, use to get a descriptor set layout that matches mMeshTexturesDescriptorSetLayout in ModelLoader
   vec4 hack = texture(normalSampler, InTex);
   vec4 hack2 = texture(specularSampler, InTex);
   float hack3 = material.ao;

   if (color.a < 0.01f)
      discard;

   OutColor = gl_FragCoord.z;
   //OutColorDebug = linearDepth(gl_FragCoord.z) / FAR_PLANE; // Use this if perspective projection matrix
}