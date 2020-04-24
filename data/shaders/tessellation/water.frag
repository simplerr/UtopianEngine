#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_water.glsl"
#include "phong_lighting.glsl"
#include "calculate_shadow.glsl"

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

layout (set = 0, binding = 0) uniform UBO_waterParameters
{
    vec3 waterColor;
    float time;
    vec3 foamColor;
    float waveSpeed;
    float foamSpeed;
    float distortionStrength;
    float shorelineDepth;
    float waveFrequnecy; // Scaling used when sampling distortion texture
} ubo_waterParameters;

layout (set = 0, binding = 2) uniform sampler2D dudvSampler;
layout (set = 0, binding = 3) uniform sampler2D normalSampler;
layout (set = 0, binding = 5) uniform sampler2D depthSampler; // Binding 4 is cascades_ubo in calculate_shadow.glsl
layout (set = 0, binding = 8) uniform sampler2D foamMaskSampler; // Binding 4 is cascades_ubo in calculate_shadow.glsl

// Further scaled with waveSpeed and foamSpeed
const float timeScaling = 0.00003f;

// Todo: Move to common file
float eye_z_from_depth(float depth, mat4 Proj)
{
   return -Proj[3][2] / (Proj[2][2] + depth);
}

// Reference: http://fire-face.com/personal/water/index.html
vec4 calculateFoam(float waterDepth)
{
    vec4 resultColor = vec4(vec3(0.0f), 1.0f);

    /* Shoreline */
    float timeOffset = ubo_waterParameters.time * timeScaling * ubo_waterParameters.foamSpeed;
    vec2 scaledprojectedUV = InTex * 950.0f;
    float channelA = texture(foamMaskSampler, scaledprojectedUV - vec2(timeOffset, cos(InTex.x))).r;
    float channelB = texture(foamMaskSampler, scaledprojectedUV * 0.5 + vec2(sin(InTex.y), timeOffset)).b;

    float mask = (channelA + channelB) * 0.95;
    mask = pow(mask, 2);
    mask = clamp(mask, 0.0f, 1.0f);

    float shorelineDepth = ubo_waterParameters.shorelineDepth;

    float leading = 1.0f;
    const float leadingEdgeFalloff = 0.2;
    if (waterDepth < shorelineDepth * leadingEdgeFalloff)
    {
        leading = waterDepth / (shorelineDepth * leadingEdgeFalloff);
        resultColor.a = leading;
        mask *= leading;
    }

    // Calculate linear falloff value
    float falloff = 1.0f - (waterDepth / shorelineDepth);

    // Color the foam, blend based on alpha
    const vec3 foamColor = vec3(0.9f);
    vec3 edge = foamColor.rgb * falloff;

    // Subtract mask value from foam gradient, then add the foam value to the final pixel color
    resultColor.rgb = clamp(edge - vec3(mask), 0.0, 1.0);
    return resultColor;
}

vec2 calculateDistortion(vec2 projectedUV, vec2 distortedTexCoords, float depthToWater)
{
    vec2 distortion = (texture(dudvSampler, distortedTexCoords).rg * 2.0f - 1.0f) * ubo_waterParameters.distortionStrength;
    float sampleDepth = texture(depthSampler, projectedUV + distortion).r;

    // If sample point is infront of water set zero distortion, see https://mtnphil.wordpress.com/2012/09/23/water-shader-part-3-deferred-rendering/.
    if (sampleDepth < gl_FragCoord.z)
        distortion = vec2(0.0f);

    // Reduce the distortion in relation to distance. Distortion in long distances causes sampling
    // outside of the reflection image in fresnel.frag, and the effect is mostly noticable in close range anyways.
    depthToWater *= -1;
    const float distortionRange = 2500.0f;
    float distanceFadeFactor = smoothstep(1.0f, 0.0f, (depthToWater - distortionRange) / distortionRange);
    distortion *= distanceFadeFactor;
    //OutAlbedo = vec4(vec3(distanceFadeFactor), 1.0f);

    return distortion;
}

void main() 
{
    vec3 worldPosition = InPosW;
    mat3 normalMatrix = transpose(inverse(mat3(ubo_camera.view)));
    vec3 normal = InNormalL;
    normal.y *= -1; // Should this be here? Similar result without it
	vec3 viewNormal = normalMatrix * normal;

    float reflectivity = 1.0f;
    OutNormal = vec4(InNormalL, 1.0f); // Missing world transform?
    viewNormal = normalize(viewNormal) * 0.5 + 0.5;
    OutNormalView = vec4(viewNormal, 1.0f);
    OutAlbedo = vec4(ubo_waterParameters.waterColor, reflectivity);
    OutPosition = vec4(worldPosition, 1.0f);

    /* Project texture coordinates */
    vec4 clipSpace = ubo_camera.projection * ubo_camera.view * vec4(InPosW.xyz, 1.0f);
    vec2 ndc = clipSpace.xy / clipSpace.w;
    vec2 projectedUV = ndc / 2 + 0.5f;

    /* Calculate distorted texture coordinates for normals and reflection/refraction distortion */
    const float textureScaling = 90.0f;
    vec2 texCoord = InTex * ubo_waterParameters.waveFrequnecy;
    float offset = ubo_waterParameters.time * timeScaling * ubo_waterParameters.waveSpeed;
    vec2 distortedTexCoords = texture(dudvSampler, vec2(texCoord.x + offset, texCoord.y)).rg * 0.1f;
    distortedTexCoords = texCoord + vec2(distortedTexCoords.x, distortedTexCoords.y + offset);

    /* Normal */
    vec4 normalMapColor = texture(normalSampler, distortedTexCoords);
    normal = vec3(normalMapColor.r * 2.0f - 1.0f, normalMapColor.b, normalMapColor.g * 2.0f - 1.0f);
    normal = normalize(normal);
    // Note: Outputting this normal means that the normal in the SSR shader is not (0,1,0) so
    // the reflection texture itself will be distorted, meaning that the distortin in fresnel.frag is "done twice"
    //OutNormal = vec4(normal, 1.0f);

    /* Shadows */
	uint cascadeIndex = 0;
	float shadow = calculateShadow(worldPosition, normal, normalize(light_ubo.lights[0].dir), cascadeIndex);

    /* Phong lighting */
    Material material;
	material.ambient = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.specular = vec4(1.0f, 1.0f, 1.f, 1.0f); 

	vec4 litColor;
	vec3 toEyeW = normalize(ubo_camera.eyePos + InPosW); // Todo: Move to common file
	ApplyLighting(material, worldPosition, normal, toEyeW, vec4(ubo_waterParameters.waterColor, 1.0f), shadow, litColor);
    OutFragColor = litColor;
    OutFragColor.a = 1;

    /* Water depth calculation */
    float depth = texture(depthSampler, projectedUV).r;
    float depthToTerrain = eye_z_from_depth(depth, ubo_camera.projection);
    float depthToWater = eye_z_from_depth(gl_FragCoord.z, ubo_camera.projection);
    float waterDepth = depthToWater - depthToTerrain;

    /* Foam effect at shoreline */
    vec4 foamColor = calculateFoam(waterDepth);
    OutFragColor.rgb += foamColor.rgb;
    OutFragColor.a = foamColor.a;

    /* Output distortion used in fresnel.frag */
    OutDistortion = calculateDistortion(projectedUV, distortedTexCoords, depthToWater);

    /* Apply wireframe */
    // Reference: http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
    if (ubo_settings.wireframe == 1)
    {
        vec3 d = fwidth(InBarycentric);
        vec3 a3 = smoothstep(vec3(0.0), d * 0.8, InBarycentric);
        float edgeFactor = min(min(a3.x, a3.y), a3.z);
        OutFragColor.rgb = mix(vec3(1.0), OutFragColor.rgb, edgeFactor);

        // Simple method but agly aliasing:
        // if(any(lessThan(InBarycentric, vec3(0.02))))
    }
}
