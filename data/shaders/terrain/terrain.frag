#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPosW;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 2) uniform sampler3D texture3d;

layout (std140, set = 0, binding = 1) uniform UBO 
{
	vec3 eyePos;
	float padding;
	float fogStart;
	float fogDistance;
} per_frame_ps;

void main(void)
{
	// Calculate the color based on height
	vec3 biomeColor = vec3(1.0f, 1.0f, 1.0f);
	
	biomeColor = mix(biomeColor, vec3(0, 1, 0), inPosW.y / 5000.0);
	biomeColor = min(biomeColor, vec3(1, 1, 1));


	vec3 lightVec = vec3(1, 1, 1);
	lightVec = normalize(lightVec);

	float diffuseFactor = max(0, dot(lightVec, inNormal));

	float ambientFactor = 0.1;
	vec3 color = biomeColor * ambientFactor + diffuseFactor * biomeColor;

	// Apply fogging.
	float distToEye = length(per_frame_ps.eyePos + inPosW); // TODO: NOTE: This should be "-". Related to the negation of the world matrix push constant.
	float fogLerp = clamp((distToEye - per_frame_ps.fogStart) / per_frame_ps.fogDistance, 0.0, 1.0); 

	// Blend the fog color and the lit color.
	color = mix(color, vec3(0.2), fogLerp);
	outFragColor = vec4(color,  1.0);
}