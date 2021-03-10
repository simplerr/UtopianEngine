#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"
#include "atmosphere_inc.glsl"

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InPosL;
layout (location = 2) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;
layout (location = 1) out vec4 OutSun;

void main()
{
   vec3 rayStart = sharedVariables.eyePos.xyz;
   vec3 rayDir = normalize(InPosL);
   float rayLength = 999999999.0f;
   vec3 sunDir = ubo_atmosphere.sunDir;
   sunDir.z *= -1;
   vec3 lightColor = vec3(1.0f);

   vec3 transmittance;
   vec3 color = IntegrateScattering(rayStart, rayDir, rayLength, sunDir, lightColor, transmittance);

   OutFragColor = vec4(vec3(color), 1.0f);

   // This is to get the input for the SunShaftJob. It's not physically correct
   // and might make the sky look weird and over expose the sun in some cases.
   float sun = 10 * pow(max(dot(sunDir, rayDir), 0.0), 1200.0);
   OutSun = vec4(vec3(sun * color), 1.0f);
}
