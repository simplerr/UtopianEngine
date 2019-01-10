#version 450

layout (location = 0) in vec3 InColor;
layout (location = 1) in vec3 InPosW;
layout (location = 2) in vec3 InNormalW;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InNormalV;
layout (location = 5) in vec2 InTextureTiling;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

// Output normals in view space so that the SSAO pass can use them.
// Should be reworked so that you don't have to use two separate textures
// for normals in world space vs view space.
layout (location = 3) out vec4 outNormalV;

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;

const float NEAR_PLANE = 10.0f; //todo: specialization const
const float FAR_PLANE = 256000.0f; //todo: specialization const 

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() 
{
	vec4 diffuse = texture(diffuseSampler, InTex * InTextureTiling);
	vec4 normal = texture(normalSampler, InTex * InTextureTiling);

	//diffuse = normal;

	if (diffuse.a < 0.01f)
		discard;

	if (InColor != vec3(1.0f))
		diffuse = vec4(InColor, 1.0f);

	outPosition = vec4(InPosW, linearDepth(gl_FragCoord.z));
	outNormal = vec4(normalize(InNormalW), 1.0f);
	outNormal.y *= -1.0f;
	outAlbedo = vec4(diffuse);
	outNormalV = vec4(normalize(InNormalV) * 0.5 + 0.5, 1.0f);
}