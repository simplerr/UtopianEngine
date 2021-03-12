#version 450
#extension GL_GOOGLE_include_directive : enable

#include "material_types.glsl"
#include "shared_variables.glsl"

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec2 InTex;

layout (location = 0) out vec2 OutThickness;

const float NEAR_PLANE = 10.0f;
const float FAR_PLANE = 256000.0f;

layout (set = 0, binding = 1) uniform sampler2D depthSampler;

layout (set = 1, binding = 0) uniform sampler2D texSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D specularSampler;

// Todo: Move to common file
float eye_z_from_depth(float depth, mat4 Proj)
{
   return -Proj[3][2] / (Proj[2][2] + depth);
}

void main()
{
   vec4 color = texture(texSampler, InTex);

   if (color.a < 0.01f)
      discard;

   // Todo: Remove, used to get a descriptor set layout that matches mMeshTexturesDescriptorSetLayout in ModelLoader
   vec4 hack = texture(normalSampler, InTex);
   vec4 hack2 = texture(specularSampler, InTex);

   /* Project texture coordinates */
   vec4 clipSpace = sharedVariables.projectionMatrix * sharedVariables.viewMatrix * vec4(InPosW.xyz, 1.0f);
   vec2 ndc = clipSpace.xy / clipSpace.w;
   vec2 projectedUV = ndc / 2 + 0.5f;

   float frontfaceDepth = -eye_z_from_depth(texture(depthSampler, projectedUV).r, sharedVariables.projectionMatrix);
   float backfaceDepth = -eye_z_from_depth(gl_FragCoord.z, sharedVariables.projectionMatrix);
   float depthDelta = backfaceDepth - frontfaceDepth;

   OutThickness.y = depthDelta;
   OutThickness.x = gl_FragCoord.z;
}