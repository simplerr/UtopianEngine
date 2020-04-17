#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_water.glsl"
#include "../common/sky_color.glsl"

layout (location = 0) in vec3 InNormalL;
layout (location = 1) in vec2 InTex;
layout (location = 2) in vec3 InTangent;
layout (location = 3) in vec3 InPosW;
layout (location = 4) in vec3 InBarycentric;

layout (location = 0) out vec4 OutFragColor;
layout (location = 1) out vec4 OutPosition;
layout (location = 2) out vec4 OutNormal;
layout (location = 3) out vec4 OutAlbedo;
layout (location = 4) out vec4 OutNormalView;
layout (location = 5) out vec2 OutDistortion;

layout (set = 0, binding = 1) uniform UBO_waterParameters
{
    float time;
} ubo_waterParameters;

layout (set = 0, binding = 0) uniform sampler2D dudvTexture;

const float distortionStrength = 1;//0.02f;

void main() 
{
    // Project texture coordinates
    vec4 clipSpace = ubo_camera.projection * ubo_camera.view * vec4(InPosW.xyz, 1.0f);
    vec2 ndc = clipSpace.xy / clipSpace.w;
    vec2 uv = ndc / 2 + 0.5f;

    // Blue water for now
    vec4 color = vec4(0.0f, 0.0f, 1.0f, 1.0f);

    // Todo: Note: the + sign is due to the fragment world position is negated for some reason
	// this is a left over from an old problem
	vec3 toEyeW = normalize(ubo_camera.eyePos + InPosW);

    vec3 reflection = reflect(toEyeW, vec3(0, 1, 0));
    reflection.y *= -1; // Note: -1
    SkyOutput testOutput = GetSkyColor(reflection);
    color = testOutput.skyColor;

    vec3 worldPosition = InPosW;
    mat3 normalMatrix = transpose(inverse(mat3(ubo_camera.view)));
    vec3 normal = InNormalL;
    normal.y *= -1; // Should this be here? Similar result without it
	vec3 viewNormal = normalMatrix * normal;

    // Apply wireframe
    // Reference: http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
    if (ubo_settings.wireframe == 1)
    {
        vec3 d = fwidth(InBarycentric);
        vec3 a3 = smoothstep(vec3(0.0), d * 0.8, InBarycentric);
        float edgeFactor = min(min(a3.x, a3.y), a3.z);
        color.rgb = mix(vec3(1.0), color.rgb, edgeFactor);

        // Simple method but agly aliasing:
        // if(any(lessThan(InBarycentric, vec3(0.02))))
    }

    float reflectivity = 1.0f;
    OutFragColor = color;
    OutNormal = vec4(InNormalL, 1.0f); // Missing world transform
    // Use: here?
    viewNormal = normalize(viewNormal) * 0.5 + 0.5;
    OutNormalView = vec4(viewNormal, 1.0f);
    OutAlbedo = vec4(0.0f, 0.0f, 1.0f, reflectivity);
    OutPosition = vec4(worldPosition, 1.0f);

    // Distortion calculation
    const float textureScaling = 10.0f;
    const float timeScaling = 0.00001f;
    vec2 texCoord = InTex * textureScaling;
    float offset = ubo_waterParameters.time * timeScaling;
    vec2 distortion1 = (texture(dudvTexture, vec2(texCoord.x + offset, texCoord.y)).rg * 2.0f - 1.0f) * distortionStrength;
    vec2 distortion2 = (texture(dudvTexture, vec2(-texCoord.x + offset, texCoord.y + offset)).rg * 2.0f - 1.0f) * distortionStrength;
    vec2 totalDistortion = distortion1 + distortion2;
    OutDistortion = totalDistortion;
}