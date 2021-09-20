layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D specularSampler;
layout (set = 1, binding = 3) uniform sampler2D metallicRoughnessSampler;

layout (std140, set = 1, binding = 20) uniform UBO_material
{
   vec4 baseColorFactor;
   float metallicFactor;
   float roughnessFactor;
   float ao;
   float pad;
} material;