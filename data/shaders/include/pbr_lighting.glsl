#extension GL_GOOGLE_include_directive : enable

#include "brdf.glsl"
#include "common_types.glsl"

struct PixelParams
{
   vec3 position;
   vec3 baseColor;
   vec3 normal;
   vec3 F0;
   float metallic;
   float roughness;
};

vec3 surfaceShading(const PixelParams pixel, const Light light, const vec3 eyePos, float lightColorFactor)
{
   vec3 color = vec3(0.0f);

   /* Implementation from https://learnopengl.com/PBR/Theory */
   vec3 N = pixel.normal;
   vec3 V = normalize(eyePos - pixel.position);
   vec3 R = reflect(V, N);

   vec3 F0 = vec3(0.04);
   F0 = mix(F0, pixel.baseColor, pixel.metallic);

   // Reflectance equation
   vec3 Lo = vec3(0.0);
   // Calculate per-light radiance
   vec3 L = normalize(light.pos - pixel.position);
   vec3 H = normalize(V + L);
   float distance    = length(light.pos - pixel.position);
   float attenuation = 1.0 / (distance * distance);
   vec3 radiance     = light.color.rgb * attenuation * lightColorFactor;

   // Cook-torrance brdf
   float NDF = DistributionGGX(N, H, pixel.roughness);
   float G   = GeometrySmith(N, V, L, pixel.roughness);
   vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

   vec3 kS = F;
   vec3 kD = vec3(1.0) - kS;
   kD *= 1.0 - pixel.metallic;

   vec3 numerator    = NDF * G * F;
   float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
   vec3 specular     = numerator / denominator;

   // Add to outgoing radiance Lo
   float NdotL = max(dot(N, L), 0.0);
   color = (kD * pixel.baseColor / PI + specular) * radiance * NdotL;

   return color;
}