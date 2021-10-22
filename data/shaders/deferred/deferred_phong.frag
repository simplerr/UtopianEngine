#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "common_types.glsl"
#include "phong_lighting.glsl"
#include "calculate_shadow.glsl"
#include "shared_variables.glsl"
#include "atmosphere/atmosphere_inc.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 1, binding = 0) uniform sampler2D positionSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D albedoSampler;
layout (set = 1, binding = 3) uniform sampler2D ssaoSampler;
layout (set = 1, binding = 4) uniform sampler2D pbrSampler;

// UBO_lights from phong_lighting.glsl is at slot = 0, binding = 1

layout (std140, set = 0, binding = 2) uniform UBO_settings
{
   vec3 fogColor;
   float fogStart;
   float fogDistance;
   int cascadeColorDebug;
   int useIBL;
} settings_ubo;

void main()
{
   vec3 position = texture(positionSampler, InTex).xyz;
   vec3 normal = texture(normalSampler, InTex).rgb;
   vec3 albedo = texture(albedoSampler, InTex).rgb;
   float specularIntensity = texture(albedoSampler, InTex).a;
   float occlusion = texture(pbrSampler, InTex).r;
   float roughness = texture(pbrSampler, InTex).g;
   float metallic = texture(pbrSampler, InTex).b;

   // From sRGB space to Linear space
   albedo.rgb = pow(albedo.rgb, vec3(2.2));

   // Todo: Note: the + sign is due to the fragment world position is negated for some reason
   // this is a left over from an old problem
   vec3 toEyeW = normalize(sharedVariables.eyePos.xyz + position);

   // Calculate shadow factor
   // Note: Assume directional light at index 0
   uint cascadeIndex = 0;
   float shadow = calculateShadow(position, normal, normalize(light_ubo.lights[0].dir), cascadeIndex);

   PhongMaterial material;
   material.ambient = vec4(1.0f, 1.0f, 1.0f, 1.0f);
   material.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
   material.specular = vec4(1.0f, 1.0f, 1.0f, 1024.0f);

   vec4 litColor;
   ApplyLighting(material, position, normal, toEyeW, vec4(albedo, 1.0f), shadow, litColor);

   // Apply fogging.
   float distToEye = length(sharedVariables.eyePos.xyz + position); // TODO: NOTE: This should be "-". Related to the negation of the world matrix push constant.
   float fogLerp = clamp((distToEye - settings_ubo.fogStart) / settings_ubo.fogDistance, 0.0, 1.0);

   // Blend the fog color and the lit color.
   litColor = vec4(mix(litColor.rgb, settings_ubo.fogColor, fogLerp), 1.0f);

   float ssao = texture(ssaoSampler, InTex).r;

   // Atmospheric scattering effect
   // This just includes the first part of the whole effect and should be extended
   // Todo: Note: Remove this if-statement
   // Reference: https://github.com/Fewes/MinimalAtmosphere
   if (ubo_atmosphere.atmosphericScattering == 1)
   {
      vec3 sunDir = ubo_atmosphere.sunDir;
      vec3 lightTransmittance = Absorb(IntegrateOpticalDepth(position, sunDir));
      litColor.rgb *= lightTransmittance;
   }

   OutFragColor = litColor * ssao;

   if (settings_ubo.cascadeColorDebug == 1)
   {
      switch(cascadeIndex) {
         case 0 :
            OutFragColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
            break;
         case 1 :
            OutFragColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
            break;
         case 2 :
            OutFragColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
            break;
         case 3 :
            OutFragColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
            break;
      }
   }
}
