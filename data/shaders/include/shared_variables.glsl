// This uniform buffer contains data that is common in multiple shaders.
// To use it simply bind gRenderer().GetSharedShaderVariables() to it.
layout (std140, set = 0, binding = 20) uniform UBO_sharedVariables
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 inverseProjectionMatrix;
	vec4 eyePos;
	vec2 viewportSize;
	vec2 mouseUV;
	float time;
} sharedVariables;