#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "phong_lighting.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 1, binding = 0) uniform sampler2D positionSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D albedoSampler;
layout (set = 1, binding = 3) uniform sampler2D ssaoSampler;
layout (set = 1, binding = 4) uniform sampler2D shadowSampler;

layout (std140, set = 0, binding = 0) uniform UBO_eyePos
{
	vec4 EyePosW;
} eye_ubo;

layout (std140, set = 0, binding = 2) uniform UBO_fog
{
	vec3 fogColor;
	float padding;
	float fogStart;
	float fogDistance;
} fog_ubo;

layout (std140, set = 0, binding = 3) uniform UBO_lightTransform
{
	mat4 viewProjection;
} light_transform;

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

	// Calculate shadow factor
	vec4 lightSpacePosition = light_transform.viewProjection * vec4(position, 1.0f);
	vec4 projCoordinate = lightSpacePosition / lightSpacePosition.w; // Perspective divide 
	projCoordinate.xy = projCoordinate.xy * 0.5f + 0.5f;

	float shadow = 1.0f;
	// If fragment depth is outside frustum do no shadowing
	if (projCoordinate.z <= 1.0f && projCoordinate.z > -1.0f)
	{
		float closestDepth = texture(shadowSampler, projCoordinate.xy).r;
		float bias = 0.00000065;//0.005;
		shadow = (projCoordinate.z - bias) > closestDepth ? 0.0f : 1.0f;
	}
	//shadow = 1;

	Material material;
	material.ambient = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.specular = vec4(1.0f, 1.0f, 1.0f, 1.0f); 

	vec4 litColor;
	ApplyLighting(material, position, normal, toEyeW, vec4(albedo, 1.0f), shadow, litColor);

	// Apply fogging.
	float distToEye = length(eye_ubo.EyePosW.xyz + position); // TODO: NOTE: This should be "-". Related to the negation of the world matrix push constant.
	float fogLerp = clamp((distToEye - fog_ubo.fogStart) / fog_ubo.fogDistance, 0.0, 1.0); 

	// Blend the fog color and the lit color.
	// Note: Disabled for now
	//litColor = vec4(mix(litColor.rgb, fog_ubo.fogColor, fogLerp), 1.0f);

	float ssao = texture(ssaoSampler, uv).r;
	ssao = 1.0;
	OutFragColor = litColor * ssao;

	// float depth = texture(positionSampler, uv).w;// / 100000.0f;
	// OutFragColor = vec4(depth, depth, depth, 1.0f);
}
