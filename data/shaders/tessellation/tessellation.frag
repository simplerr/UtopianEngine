#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared.glsl"

layout (location = 0) in vec3 InNormalL;
layout (location = 1) in vec2 InTex;
layout (location = 2) in vec3 InTangent;
layout (location = 3) in vec3 InPosW;
layout (location = 4) in vec3 InBarycentric;

layout (location = 0) out vec4 OutColor;

layout (set = 0, binding = 4) uniform sampler2D samplerDiffuse[3];
layout (set = 0, binding = 5) uniform sampler2D samplerNormal[3];

// Calculates the TBN matrix.
// From http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // Get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
 
    // Solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // Construct a scale-invariant frame 
    float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
    return mat3(T * invmax, B * invmax, N);
}

void main() 
{
    vec4 blend = texture(samplerBlendmap, InTex / ubo_settings.textureScaling);

    float textureScaling = 45.0;
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
    OutColor = vec4(finalDiffuse * diffuse, 1.0);

    // Apply wireframe
    // Reference: http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
    if (ubo_settings.wireframe == 1)
    {
        vec3 d = fwidth(InBarycentric);
        vec3 a3 = smoothstep(vec3(0.0), d * 0.8, InBarycentric);
        float edgeFactor = min(min(a3.x, a3.y), a3.z);
        OutColor.rgb = mix(vec3(1.0), OutColor.rgb, edgeFactor);

        // Simple method but agly aliasing:
        // if(any(lessThan(InBarycentric, vec3(0.02))))
    }

    // Debugging:
    //bumpNormal = bumpNormal.rbg;
    //OutColor = vec4(bumpNormal, 1.0);
    //OutColor = vec4(getNormal(InTex), 1.0);
    //OutColor = blend;
}