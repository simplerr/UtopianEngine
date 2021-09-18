#version 450

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;
layout (location = 4) in vec2 InTex;
layout (location = 5) in vec4 InTangentL;

layout (location = 0) out vec4 OutColor;

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D specularSampler;

layout (std140, set = 1, binding = 3) uniform UBO_material
{
   vec4 albedo;
   float metallic;
   float roughness;
   float ao;
   float pad;
} material;

const int numLights = 4;
vec3 lightPositions[numLights] = {
   vec3(0.0f, 1.0f, 10.0f),
   vec3(0.0f, 10.0f, 10.0f),
   vec3(10.0f, 1.0f, 10.0f),
   vec3(10.0f, 10.0f, 10.0f)
};

vec3 lightColor = vec3(150.0f);

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
   float a      = roughness*roughness;
   float a2     = a*a;
   float NdotH  = max(dot(N, H), 0.0);
   float NdotH2 = NdotH*NdotH;

   float num   = a2;
   float denom = (NdotH2 * (a2 - 1.0) + 1.0);
   denom = PI * denom * denom;

   return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
   float r = (roughness + 1.0);
   float k = (r*r) / 8.0;

   float num   = NdotV;
   float denom = NdotV * (1.0 - k) + k;

   return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
   float NdotV = max(dot(N, V), 0.0);
   float NdotL = max(dot(N, L), 0.0);
   float ggx2  = GeometrySchlickGGX(NdotV, roughness);
   float ggx1  = GeometrySchlickGGX(NdotL, roughness);

   return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
   return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
   vec4 diffuse = texture(diffuseSampler, InTex);
   vec4 normal = texture(normalSampler, InTex);
   vec4 specular = texture(specularSampler, InTex);

   if (diffuse.a < 0.5f)
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

   /* Implementation from https://learnopengl.com/PBR/Theory */
   vec3 V = normalize(InEyePosW - InPosW);
   vec3 F0 = vec3(0.04);
   F0 = mix(F0, material.albedo.rgb, material.metallic);

   // Reflectance equation
   vec3 Lo = vec3(0.0);
   for (int i = 0; i < numLights; i++)
   {
      // Calculate per-light radiance
      vec3 L = normalize(lightPositions[i] - InPosW);
      vec3 H = normalize(V + L);
      float distance    = length(lightPositions[i] - InPosW);
      float attenuation = 1.0 / (distance * distance);
      vec3 radiance     = lightColor * attenuation;

      // Cook-torrance brdf
      float NDF = DistributionGGX(N, H, material.roughness);
      float G   = GeometrySmith(N, V, L, material.roughness);
      vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

      vec3 kS = F;
      vec3 kD = vec3(1.0) - kS;
      kD *= 1.0 - material.metallic;

      vec3 numerator    = NDF * G * F;
      float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
      vec3 specular     = numerator / denominator;

      // Add to outgoing radiance Lo
      float NdotL = max(dot(N, L), 0.0);
      Lo += (kD * material.albedo.rgb / PI + specular) * radiance * NdotL;
   }

   vec3 ambient = vec3(0.03) * material.albedo.rgb * material.ao;
   vec3 color = ambient + Lo;

   color = color / (color + vec3(1.0));
   color = pow(color, vec3(1.0/2.2));
   OutColor = vec4(color, 1.0f);
}
