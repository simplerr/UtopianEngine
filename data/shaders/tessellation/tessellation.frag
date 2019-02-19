#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared.glsl"

layout (location = 0) in vec3 InNormalL;
layout (location = 1) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

layout (set = 0, binding = 4) uniform sampler2D samplerDiffuse;
layout (set = 0, binding = 5) uniform sampler2D samplerNormal;

void main() 
{
    float textureScaling = 45.0;
    vec3 diffuseTexture = texture(samplerDiffuse, InTex * textureScaling).xyz;
    vec3 normalTexture = texture(samplerNormal, InTex * textureScaling).xyz;
    //vec3 displacementTexture = texture(samplerDisplacement, InTex * textureScaling).xyz;
    vec3 normal = getNormal(InTex);
    normal = normalTexture.rbg;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 lightDir = vec3(sin(ubo_camera.time / 600.0), 1, 1);
    float diffuse = dot(normal, normalize(lightDir)); 
    OutColor = vec4(diffuseTexture * diffuse, 1.0);

    // OutColor = vec4(diffuseTexture, 1.0);
    // OutColor = vec4(normalTexture, 1.0);
    //OutColor = vec4(displacementTexture, 1.0);
    //OutColor = vec4(normal, 1.0);
}