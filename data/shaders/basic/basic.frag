#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPosW;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

layout (std140, set = 0, binding = 1) uniform UBO 
{
	vec3 eyePos;
	float padding;
	float fogStart;
	float fogDistance;
} per_frame;

void main(void)
{
	vec3 color = vec3(1, 1, 1);
	vec3 lightVec = vec3(1, 1, 1);
	lightVec = normalize(lightVec);

	float diffuseFactor = max(0, dot(lightVec, inNormal));

	float ambientFactor = 0.1;
	outFragColor = vec4(color * ambientFactor + diffuseFactor * color, 1.0);

	outFragColor = vec4(inNormal.r, inNormal.g, inNormal.b, 1.0);
	//outFragColor = vec4(0, 0, inNormal.b, 1.0);

	// Apply fogging.
	float distToEye = length(per_frame.eyePos + inPosW); // TODO: NOTE: This should be "-". Related to the negation of the world matrix push constant.
	float fogLerp = clamp((distToEye - per_frame.fogStart) / per_frame.fogDistance, 0.0, 1.0); 

	// Blend the fog color and the lit color.
	vec3 litColor = mix(inColor, vec3(0.5), fogLerp);
	outFragColor = vec4(litColor,  1.0);
}