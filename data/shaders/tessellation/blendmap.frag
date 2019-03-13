#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

// This should probably not be used in this effect
// Only include shared.glsl in the terrain pipeline
//#include "shared.glsl"

layout (location = 0) in vec2 InTex;
layout (location = 0) out vec4 OutFragColor;

layout (std140, set = 0, binding = 0) uniform UBO_settings 
{
    float amplitudeScaling;
} ubo_settings;

layout (set = 0, binding = 1) uniform sampler2D samplerHeightmap;
layout (set = 0, binding = 2) uniform sampler2D samplerNormalmap;

// Creates a blendmap used for texturing the terrain.
// Texture smoothing is accomplished by letting tessellation.frag sample 
// from a low resolution texture
void main() 
{
    float height = texture(samplerHeightmap, InTex).r * ubo_settings.amplitudeScaling;
    vec3 normal = normalize(texture(samplerNormalmap, InTex).xyz);

    vec3 blendmap = vec3(0.0);

    // Altitde 
    float highAltitude = 3500.0;
    vec3 flatBlend = vec3(1, 0, 0); // grass
    if (height > highAltitude)
        flatBlend = vec3(0, 0, 1);  // dirt

    // Slope
    if (normal.y > 0.70)
        blendmap = flatBlend;
    else
        blendmap.g = 1.0;

    OutFragColor = vec4(blendmap, 1.0f);
}