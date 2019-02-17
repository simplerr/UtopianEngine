/** Contains descriptors shared between multiple shader stages. */

layout (std140, set = 0, binding = 0) uniform UBO_viewProjection 
{
	mat4 projection;
	mat4 view;
} ubo_camera;

layout (std140, set = 0, binding = 1) uniform UBO_settings
{
    vec2 viewportSize;
    float edgeSize; // The size in pixels that all edges should have
	float tessellationFactor;
    float amplitude;
    float textureScaling;
} ubo_settings;

layout (set = 0, binding = 2) uniform sampler2D samplerHeightmap;
layout (set = 0, binding = 3) uniform sampler2D samplerNormalmap;

float getHeight(vec2 texCoord)
{
    float height = texture(samplerHeightmap, texCoord / ubo_settings.textureScaling).r * ubo_settings.amplitude;

    return height;
}

vec3 getNormal(vec2 texCoord)
{
    vec3 normal = texture(samplerNormalmap, texCoord / ubo_settings.textureScaling).xyz;

    return normal;
}