#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_water.glsl"
#include "phong_lighting.glsl"

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
    float time;
} ubo_waterParameters;

layout (set = 0, binding = 2) uniform sampler2D dudvSampler;
layout (set = 0, binding = 3) uniform sampler2D normalSampler;
layout (set = 0, binding = 4) uniform sampler2D depthSampler;

const float distortionStrength = 0.05f;

const float NEAR_PLANE = 10.0f; //todo: specialization const
const float FAR_PLANE = 256000.0f; //todo: specialization const 

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

float eye_z_from_depth(float depth, mat4 Proj)
{
   return -Proj[3][2] / (Proj[2][2] + depth);
}

void main() 
{
    // Project texture coordinates
    vec4 clipSpace = ubo_camera.projection * ubo_camera.view * vec4(InPosW.xyz, 1.0f);
    vec2 ndc = clipSpace.xy / clipSpace.w;
    vec2 uv = ndc / 2 + 0.5f;

    // Blue water for now
    vec4 color = vec4(0.0f, 0.1f, 0.4f, 1.0f);

    // Todo: Note: the + sign is due to the fragment world position is negated for some reason
	// this is a left over from an old problem
	vec3 toEyeW = normalize(ubo_camera.eyePos + InPosW);

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
    // vec2 distortion1 = (texture(dudvSampler, vec2(texCoord.x + offset, texCoord.y)).rg * 2.0f - 1.0f) * distortionStrength;
    // vec2 distortion2 = (texture(dudvSampler, vec2(-texCoord.x + offset, texCoord.y + offset)).rg * 2.0f - 1.0f) * distortionStrength;
    // vec2 totalDistortion = distortion1 + distortion2;

    // Calculate distorted texture coordinates for normals and reflection/refraction distortion
    const float textureScaling = 50.0f;
    const float timeScaling = 0.00003f;
    vec2 texCoord = InTex * textureScaling;
    float offset = ubo_waterParameters.time * timeScaling;
    vec2 distortedTexCoords = texture(dudvSampler, vec2(texCoord.x + offset, texCoord.y)).rg * 0.1f;
    distortedTexCoords = texCoord + vec2(distortedTexCoords.x, distortedTexCoords.y + offset);

    // Normal
    vec4 normalMapColor = texture(normalSampler, distortedTexCoords);
    normal = vec3(normalMapColor.r * 2.0f - 1.0f, normalMapColor.b, normalMapColor.g * 2.0f - 1.0f);
    normal = normalize(normal);
    // Note: Outputting this normal means that the normal in the SSR shader is not (0,1,0) so
    // the reflection texture itself will be distorted, meaning that the distortin in fresnel.frag is "done twice"
    //OutNormal = vec4(normal, 1.0f);

    Material material;
	material.ambient = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.specular = vec4(1.0f, 1.0f, 1.f, 1.0f); 

    float shadow = 1.0f;
	vec4 litColor;
	ApplyLighting(material, worldPosition, normal, toEyeW, vec4(color.rgb, 1.0f), shadow, litColor);
    OutFragColor = litColor;
    OutAlbedo = litColor;

    // Shoreline
    float depth = texture(depthSampler, uv).r;
    float depthToTerrain = eye_z_from_depth(depth, ubo_camera.projection);
    float depthToWater = eye_z_from_depth(gl_FragCoord.z, ubo_camera.projection);
    float waterDepth = depthToWater - depthToTerrain;
    const float shorelineDepth = 100.0f;
    if (waterDepth < shorelineDepth)
        OutFragColor = vec4(0.7f);

    // Output distorting used in fresnel.frag
    vec2 outputDistortion = (texture(dudvSampler, distortedTexCoords).rg * 2.0f - 1.0f) * distortionStrength;
    OutDistortion = outputDistortion;
}