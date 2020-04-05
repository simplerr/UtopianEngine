#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_water.glsl"
#include "../common/sky_color.glsl"
#include "ssr_vkdf.glsl"

layout (location = 0) in vec3 InNormalL;
layout (location = 1) in vec2 InTex;
layout (location = 2) in vec3 InTangent;
layout (location = 3) in vec3 InPosW;
layout (location = 4) in vec3 InBarycentric;

layout (location = 0) out vec4 OutFragColor;

void main() 
{
   /* Use all inputs so they don't get optimized away */
   vec4 tmp = texture(lightSampler, InTex);
   tmp = texture(specularSampler, InTex);
   tmp = texture(normalWorldSampler, InTex);
   tmp = texture(positionSampler, InTex);

    // Blue water for now
    vec4 color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    color.rgb = vec3(distance(InPosW, vec3(-ubo_camera.eyePos.x, 0, -ubo_camera.eyePos.z)) / 5000.0f);

    // Todo: Note: the + sign is due to the fragment world position is negated for some reason
	// this is a left over from an old problem
	vec3 toEyeW = normalize(ubo_camera.eyePos + InPosW);
    color.rgb = vec3(toEyeW);

    vec3 reflection = reflect(toEyeW, vec3(0, 1, 0));
    reflection.y *= -1; // Note: -1
    SkyOutput testOutput = GetSkyColor(reflection);
    color = testOutput.skyColor;

    // Project texture coordinates
    vec4 clipSpace = ubo_camera.projection * ubo_camera.view * vec4(InPosW.xyz, 1.0f);
    vec2 ndc = clipSpace.xy / clipSpace.w;
    vec2 uv = ndc / 2 + 0.5f;

    // Testing transparency
    vec3 transparentColor = texture(lightSampler, uv).rgb;
    vec3 combinedColor = mix(color.rgb, transparentColor, 0.5f);
    OutFragColor = vec4(combinedColor, 1.0f);
    return;
    // End test

    float reflectiveness = 1.0f;
    vec3 worldPosition = InPosW;
    mat3 normalMatrix = transpose(inverse(mat3(ubo_camera.view)));
    vec3 normal = InNormalL;
    //normal.y *= -1;
	vec3 viewNormal = normalMatrix * normal;
    // viewNormal = normalize(viewNormal * 0.5 + 0.5);
    // viewNormal = normalize(viewNormal * 2 - 1);

    vec4 reflectionColor = retrieveReflectionColor(worldPosition, viewNormal, uv, reflectiveness);

    color = vec4(reflectionColor.rgb, 1.0f);

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

    OutFragColor = color;
    //OutFragColor.rgb = viewNormal;
}