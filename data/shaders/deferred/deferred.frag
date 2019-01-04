#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "phong_lighting.glsl"

#define SHADOW_MAP_CASCADE_COUNT 4

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 1, binding = 0) uniform sampler2D positionSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D albedoSampler;
layout (set = 1, binding = 3) uniform sampler2D ssaoSampler;
layout (set = 1, binding = 4) uniform sampler2DArray shadowSampler;

layout (std140, set = 0, binding = 0) uniform UBO_eyePos
{
	vec4 EyePosW;
} eye_ubo;

// UBO_lights from phong_lighting.glsl is at slot = 0, binding = 1

layout (std140, set = 0, binding = 2) uniform UBO_settings
{
	vec3 fogColor;
	float padding;
	float fogStart;
	float fogDistance;
	int shadowSampleSize;
	int cascadeColorDebug;
} settings_ubo;

layout (std140, set = 0, binding = 3) uniform UBO_lightTransform
{
	mat4 viewProjection;
} light_transform;

layout (std140, set = 0, binding = 4) uniform UBO_cascades
{
	vec4 cascadeSplits;
	mat4 cascadeViewProjMat[4];
	mat4 cameraViewMat;
} cascades_ubo;

float calculateShadow(vec3 position, vec3 normal, uint cascadeIndex)
{
	vec4 lightSpacePosition = light_transform.viewProjection * vec4(position, 1.0f);
	lightSpacePosition = cascades_ubo.cascadeViewProjMat[cascadeIndex] * vec4(position, 1.0f);
	vec4 projCoordinate = lightSpacePosition / lightSpacePosition.w; // Perspective divide 
	projCoordinate.xy = projCoordinate.xy * 0.5f + 0.5f;

	// Note: Todo: Assumes that the directional light is at index 0 
	vec3 lightDir = normalize(light_ubo.lights[0].dir);

	float shadow = 0.0f;
	vec2 texelSize = 1.0 / textureSize(shadowSampler, 0).xy;
	int count = 0;
	int range = settings_ubo.shadowSampleSize;
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			// If fragment depth is outside frustum do no shadowing
			if (projCoordinate.z <= 1.0f && projCoordinate.z > -1.0f)
			{
				vec2 offset = vec2(x, y) * texelSize;
				float closestDepth = texture(shadowSampler, vec3(projCoordinate.xy + offset, cascadeIndex)).r;
				float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.00000065); 
				shadow += ((projCoordinate.z - bias) > closestDepth ? 0.0f : 1.0f);
			}
			else
			{
				shadow += 1.0f;
			}

			count++;
		}
	}

	shadow /= (count);

	return shadow;
}
void main() 
{
	vec2 uv = InTex;
	uv.x *= -1;

	vec3 position = texture(positionSampler, uv).xyz;
	vec3 normal = texture(normalSampler, uv).rgb;
	vec3 albedo = texture(albedoSampler, uv).rgb;

	// Todo: Note: the + sign is due to the fragment world position is negated for some reason
	// this is a left over from an old problem
	vec3 toEyeW = normalize(eye_ubo.EyePosW.xyz + position);

	// Get cascade index for the current fragment's view position
	vec3 viewPosition = (cascades_ubo.cameraViewMat * vec4(position, 1.0f)).xyz;
	uint cascadeIndex = 0;
	for(uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
		if(viewPosition.z < cascades_ubo.cascadeSplits[i]) {	
			cascadeIndex = i + 1;
		}
	}

	// Calculate shadow factor
	float shadow = calculateShadow(position, normal, cascadeIndex);

	Material material;
	material.ambient = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.specular = vec4(1.0f, 1.0f, 1.0f, 1.0f); 

	vec4 litColor;
	ApplyLighting(material, position, normal, toEyeW, vec4(albedo, 1.0f), shadow, litColor);

	// Apply fogging.
	float distToEye = length(eye_ubo.EyePosW.xyz + position); // TODO: NOTE: This should be "-". Related to the negation of the world matrix push constant.
	float fogLerp = clamp((distToEye - settings_ubo.fogStart) / settings_ubo.fogDistance, 0.0, 1.0); 

	// Blend the fog color and the lit color.
	// Note: Disabled for now
	//litColor = vec4(mix(litColor.rgb, settings_ubo.fogColor, fogLerp), 1.0f);

	float ssao = texture(ssaoSampler, uv).r;
	OutFragColor = litColor * ssao;

	if (settings_ubo.cascadeColorDebug == 1)
	{
		switch(cascadeIndex) {
			case 0 : 
				OutFragColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
				break;
			case 1 : 
				OutFragColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
				break;
			case 2 : 
				OutFragColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
				break;
			case 3 : 
				OutFragColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
				break;
		}
	}
}
