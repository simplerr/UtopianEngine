#version 450
#extension GL_GOOGLE_include_directive : enable

#include "material.glsl"
#include "pbr_lighting.glsl"

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;
layout (location = 4) in vec2 InTex;
layout (location = 5) in vec4 InTangentL;

layout (location = 0) out vec4 OutColor;

layout (set = 2, binding = 0) uniform samplerCube irradianceMap;
layout (set = 2, binding = 1) uniform samplerCube specularMap;
layout (set = 2, binding = 2) uniform sampler2D brdfLut;

layout (std140, set = 3, binding = 0) uniform UBO_settings
{
   int debugChannel;
   int useIBL;
} per_frame_fs;


vec4 lightColor = vec4(vec3(150.0f), 1.0f);
const int numLights = 4;

Light lights[numLights] = {
   Light(lightColor, vec3(0.0f, 1.0f, 10.0f), 0.0f, vec3(0.0f), 0.0f, vec3(0,0,1), 1.0f, vec3(0.0f), 0.0f, vec4(0.0f)),
   Light(lightColor, vec3(0.0f, 10.0f, 10.0f), 0.0f, vec3(0.0f), 0.0f, vec3(0,0,1), 1.0f, vec3(0.0f), 0.0f, vec4(0.0f)),
   Light(lightColor, vec3(10.0f, 1.0f, 10.0f), 0.0f, vec3(0.0f), 0.0f, vec3(0,0,1), 1.0f, vec3(0.0f), 0.0f, vec4(0.0f)),
   Light(lightColor, vec3(10.0f, 10.0f, 10.0f), 0.0f, vec3(0.0f), 0.0f, vec3(0,0,1), 1.0f, vec3(0.0f), 0.0f, vec4(0.0f))
};

void main()
{
   vec4 baseColor = texture(diffuseSampler, InTex);
   vec4 normal = texture(normalSampler, InTex);
   vec4 specularRemoveThis = texture(specularSampler, InTex);
   float metallic = texture(metallicRoughnessSampler, InTex).b;
   float roughness = texture(metallicRoughnessSampler, InTex).g;
   float ambientOcclusion = texture(occlusionSampler, InTex).r;

   // From sRGB space to Linear space
   baseColor.rgb = pow(baseColor.rgb, vec3(2.2));

   baseColor *= material.baseColorFactor;
   metallic *= material.metallicFactor;
   roughness *= material.roughnessFactor;
   ambientOcclusion *= material.occlusionFactor;

   if (baseColor.a < 0.5f)
      discard;

   // Calculate normal
   vec3 N = normalize(InNormalW);
   if (InTangentL.xyz != vec3(0.0f))
   {
      vec3 T = normalize(InTangentL.xyz);
      vec3 B = cross(InNormalW, InTangentL.xyz) * InTangentL.w;
      mat3 TBN = mat3(T, B, N);
      N = TBN * normalize(normal.xyz * 2.0 - vec3(1.0));
   }

   PixelParams pixel;
   pixel.position = InPosW;
   pixel.baseColor = baseColor.rgb;
   pixel.normal = N;
   pixel.metallic = metallic;
   pixel.roughness = roughness;

   /* Direct lighting */
   vec3 Lo = vec3(0.0);
   for (int i = 0; i < numLights; i++)
   {
      Lo += surfaceShading(pixel, lights[i], InEyePosW, 1.0f);
   }

   /* Indirect IBL lighting */
   vec3 V = normalize(InEyePosW - InPosW);
   vec3 R = reflect(V, N);

   vec3 F0 = vec3(0.04);
   F0 = mix(F0, baseColor.rgb, metallic);

   vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
   vec3 kS = F;
   vec3 kD = 1.0 - kS;
   kD *= 1.0 - metallic;

   vec3 irradiance = texture(irradianceMap, -N).rgb;
   vec3 diffuse    = irradiance * baseColor.rgb;

   // Sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
   const float MAX_REFLECTION_LOD = 7.0;
   vec3 prefilteredColor = textureLod(specularMap, R, roughness * MAX_REFLECTION_LOD).rgb;
   vec2 brdf = texture(brdfLut, vec2(max(dot(N, V), 0.0), roughness)).rg;
   vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

   vec3 ambient = (kD * diffuse + specular) * ambientOcclusion;

   if (per_frame_fs.useIBL == 0)
   {
      ambient = vec3(0.03) * baseColor.rgb * ambientOcclusion;
   }

   vec3 color = ambient + Lo;

   /* Tonemapping */
   color = color / (color + vec3(1.0));
   color = pow(color, vec3(1.0/2.2));
   OutColor = vec4(color, 1.0f);

   int debugChannel = per_frame_fs.debugChannel;

   if (debugChannel == 1)
      OutColor = vec4(baseColor.rgb, 1.0);
   if (debugChannel == 2)
      OutColor = vec4(vec3(metallic), 1.0);
   else if (debugChannel == 3)
      OutColor = vec4(vec3(roughness), 1.0);
   else if (debugChannel == 4)
      OutColor = vec4(N, 1.0);
   else if (debugChannel == 5)
      OutColor = vec4(normalize(InTangentL.xyz), 1.0);
   else if (debugChannel == 6)
      OutColor = vec4(vec3(ambientOcclusion), 1.0);
   else if (debugChannel == 7)
      OutColor = vec4(irradiance, 1.0);
   else if (debugChannel == 8)
      OutColor = vec4(ambient, 1.0);
   else if (debugChannel == 9)
      OutColor = vec4(prefilteredColor, 1.0);
}
