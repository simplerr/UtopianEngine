#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "ssr_vkdf.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

void main()
{
   /* Use all inputs so they don't get optimized away */
   vec4 tmp = texture(lightSampler, InTex);

   /* Bail out if this fragment is not reflective */
   vec3 normal = texture(normalWorldSampler, InTex).rgb;
   vec4 specular = texture(specularSampler, InTex);
   float reflectiveness = specular.a;

   if (reflectiveness < 0.4f || normal.y < 0.7f)
   {
      OutFragColor = vec4(0.0);
      return;
   }

   vec3 worldPosition = texture(positionSampler, InTex).xyz;
   vec3 viewNormal = normalize(texture(normalViewSampler, InTex).xyz * 2.0f - 1.0f);
   vec4 reflectionColor = retrieveReflectionColor(worldPosition, viewNormal, InTex, reflectiveness);

   OutFragColor = vec4(reflectionColor.rgb, 1.0f);
   //OutFragColor.rgb = vec3(InTex, 0);
}