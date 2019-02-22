#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared.glsl"

layout (location = 0) in vec2 InTex;
layout (location = 0) out vec4 OutFragColor;

// Creates a blendmap used for texturing the terrain.
// Texture smoothing is accomplished by letting tessellation.frag sample 
// from a low resolution texture
void main() 
{
    float height = texture(samplerHeightmap, InTex).r * ubo_settings.amplitude;
    vec3 normal = normalize(texture(samplerNormalmap, InTex).xyz);

    vec4 blendmap = vec4(0.0);

    // Altitde 
    float highAltitude = 1000.0;
	if (height > highAltitude) {
        blendmap.g = 1.0;
    }
	else {
        blendmap.r = 1.0;
    }

    // Slope
    if (normal.y < 0.70)
        blendmap.b = 1.0;

    OutFragColor = blendmap;
}