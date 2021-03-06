/** Contains descriptors shared between multiple shader stages. */

#define GRASS_TEXTURE_SCALE 1.0f
#define ROCK_TEXTURE_SCALE 5.0f
#define DIRT_TEXTURE_SCALE 1.0f
#define ROAD_TEXTURE_SCALE 1.0f

#define GRASS_AMPLITUDE_SCALE 1.0f
#define ROCK_AMPLITUDE_SCALE 10.0f
#define DIRT_AMPLITUDE_SCALE 1.0f
#define ROAD_AMPLITUDE_SCALE 10.0f

layout (std140, set = 0, binding = 0) uniform UBO_frustum
{
   vec4 frustumPlanes[6];
} ubo_frustum;

layout (std140, set = 0, binding = 1) uniform UBO_settings
{
   vec2 viewportSize;
   float edgeSize; // The size in pixels that all edges should have
   float tessellationFactor;
   float amplitude;
   float textureScaling;
   float bumpmapAmplitude;
   int wireframe;
} ubo_settings;

layout (set = 0, binding = 2) uniform sampler2D samplerHeightmap;
layout (set = 0, binding = 3) uniform sampler2D samplerNormalmap;
layout (set = 0, binding = 7) uniform sampler2D samplerBlendmap;

layout (set = 0, binding = 6) uniform sampler2D samplerDisplacement[4];

float getHeight(vec2 texCoord)
{
   float height = texture(samplerHeightmap, texCoord).r * ubo_settings.amplitude;

   return height;
}

vec3 getNormal(vec2 texCoord)
{
   vec3 normal = texture(samplerNormalmap, texCoord).xyz;
   //normal = vec3(0, 1, 0);

   return normal;
}
