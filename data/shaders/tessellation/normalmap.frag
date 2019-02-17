#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "noise.glsl"

layout (set = 0, binding = 0) uniform sampler2D samplerHeightmap;

layout (location = 0) in vec2 InTex;
layout (location = 0) out vec4 OutFragColor;

// Calculates normal using a Sobel filter.
// Uses the algorithm from Oreon Engine (https://www.youtube.com/watch?v=-O5kv5GLFN4)
// and https://gamedev.stackexchange.com/questions/165575/calculating-normal-map-from-height-map-using-sobel-operator.
vec3 calculateNormal(vec2 texCoord)
{
    float texelSize = 1.0f / textureSize(samplerHeightmap, 0).y; // Assume quadratic texture size for now

    float z0 = texture(samplerHeightmap, texCoord + vec2(-texelSize, -texelSize)).r;
    float z1 = texture(samplerHeightmap, texCoord + vec2(0.0, -texelSize)).r;
    float z2 = texture(samplerHeightmap, texCoord + vec2(texelSize, -texelSize)).r;
    float z3 = texture(samplerHeightmap, texCoord + vec2(-texelSize, 0.0)).r;
    float z4 = texture(samplerHeightmap, texCoord + vec2(texelSize, 0.0)).r;
    float z5 = texture(samplerHeightmap, texCoord + vec2(-texelSize, texelSize)).r;
    float z6 = texture(samplerHeightmap, texCoord + vec2(0.0, texelSize)).r;
    float z7 = texture(samplerHeightmap, texCoord + vec2(texelSize, texelSize)).r;

    float strength = 20.0;
    vec3 normal = vec3(0.0f);

    normal.x = z0 + 2 * z3 + z5 - z2 - 2 * z4 - z7;
    normal.y = 1.0 / strength;
    normal.z = z0 + 2 * z1 + z2 - z5 - 2 * z6 - z7;

    normal = normalize(normal);

    return normal;
}

void main() 
{
    float height = texture(samplerHeightmap, InTex).r;
    vec3 normal = calculateNormal(InTex);
    OutFragColor = vec4(normal, 1);
}