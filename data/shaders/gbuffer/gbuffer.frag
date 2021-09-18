#version 450
#extension GL_GOOGLE_include_directive : enable

#include "material_types.glsl"

layout (location = 0) in vec4 InColor;
layout (location = 1) in vec3 InPosW;
layout (location = 2) in vec3 InNormalW;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InNormalV;
layout (location = 5) in vec2 InTextureTiling;
layout (location = 6) in mat3 InTBN;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

// Output normals in view space so that the SSAO pass can use them.
// Should be reworked so that you don't have to use two separate textures
// for normals in world space vs view space.
layout (location = 3) out vec4 outNormalV;
layout (location = 4) out vec4 outSpecular;

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

layout (std140, set = 0, binding = 1) uniform UBO_settings
{
   int normalMapping;
} settings_ubo;

const float NEAR_PLANE = 10.0f; //todo: specialization const
const float FAR_PLANE = 256000.0f; //todo: specialization const 

float linearDepth(float depth)
{
   float z = depth * 2.0f - 1.0f; 
   return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

void main()
{
   float hack = material.ao;

   vec4 diffuse = texture(diffuseSampler, InTex * InTextureTiling);
   vec4 specular = texture(specularSampler, InTex * InTextureTiling);

   if (diffuse.a < 0.01f)
      discard;
   
   // Multiply with the brightness of the mesh
   diffuse.rgb *= InColor.a;

   outPosition = vec4(InPosW, linearDepth(gl_FragCoord.z));

   if (settings_ubo.normalMapping == 1)
   {
      vec3 normal = texture(normalSampler, InTex * InTextureTiling).rgb;

      // Transform normal from tangent to world space
      normal = normalize(normal * 2.0 - 1.0);
      normal = normalize(InTBN * normal);

      outNormal = vec4(normalize(normal), 1.0f);
   }
   else
   {
      outNormal = vec4(normalize(InNormalW), 1.0f);
   }

   outNormal.y *= -1.0f;
   outAlbedo = vec4(diffuse.rgb, 1.0f);
   outNormalV = vec4(normalize(InNormalV) * 0.5 + 0.5, 1.0f);
   outSpecular = vec4(specular.r, MATERIAL_TYPE_OBJECT, 0, 0);
}