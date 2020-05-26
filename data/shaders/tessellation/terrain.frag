#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared.glsl"
#include "math.glsl"
#include "material_types.glsl"
#include "shared_variables.glsl"

layout (location = 0) in vec3 InNormalL;
layout (location = 1) in vec2 InTex;
layout (location = 2) in vec3 InTangent;
layout (location = 3) in vec3 InPosW;
layout (location = 4) in vec3 InBarycentric;

// GBuffer output attachments
layout (location = 0) out vec4 OutPosition;
layout (location = 1) out vec4 OutNormal;
layout (location = 2) out vec4 OutAlbedo;
layout (location = 4) out vec4 OutSpecular;

// Output normals in view space so that the SSAO pass can use them.
// Should be reworked so that you don't have to use two separate textures
// for normals in world space vs view space.
layout (location = 3) out vec4 OutNormalV;

layout (std140, set = 0, binding = 8) uniform UBO_brush 
{
    vec2 pos;
    float radius;
    float strength;
    int mode; // 0 = height, 1 = blend, 2 = foliage
    int operation; // 0 = add, 1 = remove
} ubo_brush;

layout (set = 0, binding = 4) uniform sampler2D samplerDiffuse[3];
layout (set = 0, binding = 5) uniform sampler2D samplerNormal[3];

void main()
{
    vec4 blend = texture(samplerBlendmap, InTex);
	blend = clamp(blend, vec4(0.0), vec4(1.0));

    float textureScaling = ubo_settings.textureScaling;
	vec3 lowAltitudeDiffuse = texture(samplerDiffuse[0], InTex * textureScaling).xyz; 
	vec3 highAltitudeDiffuse = texture(samplerDiffuse[1], InTex * textureScaling).xyz; 
	vec3 cliffDiffuse = texture(samplerDiffuse[2], InTex * textureScaling).xyz; 
	vec3 finalDiffuse = vec3(0.0);

	vec3 lowAltitudeNormal = texture(samplerNormal[0], InTex * textureScaling).xyz; 
	vec3 highAltitudeNormal = texture(samplerNormal[1], InTex * textureScaling).xyz; 
	vec3 cliffNormal = texture(samplerNormal[2], InTex * textureScaling).xyz; 
	vec3 finalNormal = vec3(0.0);

    finalDiffuse = blend.r * lowAltitudeDiffuse + blend.g * highAltitudeDiffuse + blend.b * cliffDiffuse;
    finalNormal = blend.r * lowAltitudeNormal + blend.g * highAltitudeNormal + blend.b * cliffNormal;
    finalNormal = normalize(finalNormal);

    // Blendmap visualization
    //finalDiffuse = blend.xyz;

    // Gets normal from normal map
    // Note: Todo: When changing ubo_settings.amplitude the normal does not get updated as expected
    // The solution is probably to regenerate the height and normal maps used to sample from.
    vec3 normal = getNormal(InTex);

    // Todo: Note: The calculation of the TBN matrix is currently performed here in 
    // the fragment shader. It should be possible to calculate it in the geometry shader
    // but the result so far does not seem to be correct.
    // Uncomment the line above to use the TBN from the geometry shader.
	//vec3 bitangent = normalize(cross(InTangent, normalize(normal)));
    //mat3 TBN = mat3(InTangent, bitangent, normal);
    mat3 TBN = cotangent_frame(normal, InPosW, InTex);

    vec3 bumpNormal = normalize(finalNormal.rgb * 2.0 - 1.0);
    bumpNormal = normalize(TBN * bumpNormal);

    vec3 lightDir = vec3(0.5, 1, 1);
    float diffuse = max(0.1, dot(bumpNormal, normalize(lightDir)) * 1.2); 
    vec4 color = vec4(finalDiffuse, 0.0);

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

    // GBuffer
    OutPosition = vec4(InPosW, 1.0);
    OutAlbedo = color;
    bumpNormal.xz *= -1; // To make the normals align with the rest of the world
    OutNormal = vec4(bumpNormal, 1.0);

    bumpNormal.y *= -1; // Unclear why this is needed
    mat3 normalMatrix = transpose(inverse(mat3(sharedVariables.viewMatrix)));
	OutNormalV = vec4(normalMatrix * bumpNormal, 1.0);
    OutNormalV.xyz = normalize(OutNormalV.xyz * 0.5 + 0.5);

    // Overlay that shows the area effect of the terrain brush
    float dist = distance(InTex, ubo_brush.pos);
    if ((dist > ubo_brush.radius - 0.0005) && dist < ubo_brush.radius)
    {
        if (ubo_brush.mode == 0)    // Height
            OutAlbedo = vec4(1.0f, 0.0f, 0.0f, 0.0f);
        if (ubo_brush.mode == 1)    // Blend
            OutAlbedo = vec4(0.0f, 1.0f, 0.0f, 0.0f);
        if (ubo_brush.mode == 2)    // Vegetation
            OutAlbedo = vec4(0.0f, 0.0f, 1.0f, 0.0f);
    }
    // Debugging:
    //bumpNormal = bumpNormal.rbg;
    //OutColor = vec4(bumpNormal, 1.0);
    //OutColor = vec4(getNormal(InTex), 1.0);
    //OutColor = blend;
    //OutColor = vec4(InTex.x, InTex.y, 0, 1);

	OutSpecular = vec4(0.0f, MATERIAL_TYPE_TERRAIN, 0.0f, 0.0f);
}