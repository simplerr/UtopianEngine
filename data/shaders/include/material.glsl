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