#version 450

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 1, binding = 0) uniform sampler2D positionSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D albedoSampler;

layout (std140, set = 0, binding = 0) uniform UBO 
{
	vec4 EyePosW;
} per_frame_ps;

void main() 
{
	vec2 uv = InTex;
	uv.x *= -1;

	vec3 position = texture(positionSampler, uv).rgb;
	vec3 normal = texture(normalSampler, uv).rgb;
	vec3 albedo = texture(albedoSampler, uv).rgb;

	OutFragColor = vec4(normal + albedo, 1.0f);
	//OutFragColor = vec4(0, InTex.x, 0, 1);
}