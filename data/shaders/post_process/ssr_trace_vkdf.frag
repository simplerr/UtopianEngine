#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "ssr_vkdf.glsl"
#include "../common/sky_color.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (std140, set = 0, binding = 9) uniform UBO_settings 
{
	int ssrEnabled;
} settings_ubo;

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
   bool ssrFound = false;
   vec4 reflectionColor = retrieveReflectionColor(worldPosition, viewNormal, InTex, reflectiveness, ssrFound);

   // No SSR reflection found or disabled, sample the skydome as a fallback method
   if (settings_ubo.ssrEnabled != 1 || !ssrFound)
   {
      vec3 toEyeW = normalize(ubo_parameters.eyePos + worldPosition); // Todo: Note: the + sign is due to the fragment world position is negated for some reason
      vec3 worldNormal = normalize(texture(normalWorldSampler, InTex).xyz);

      vec3 reflection = reflect(toEyeW, worldNormal);
      reflection.y *= -1; // Note: -1
      SkyOutput testOutput = GetSkyColor(reflection);
      OutFragColor = testOutput.skyColor;
      //OutFragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
   }
   else
   {
      OutFragColor = vec4(reflectionColor.rgb, 1.0f);
   }
}