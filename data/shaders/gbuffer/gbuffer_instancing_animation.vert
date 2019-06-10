#version 450

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

// Instancing input
layout (location = 6) in mat4 InInstanceWorld;

struct Sphere
{
	vec3 position;
	float radius;
};

layout (std140, set = 0, binding = 0) uniform UBO_viewProjection 
{
	// Camera 
	mat4 projection;
	mat4 view;
} per_frame_vs;

layout (std140, set = 0, binding = 2) uniform UBO_animationParameters 
{
	float time;
	float terrainSize; // Used to calculate windmap UV coordinate
	float strength;
	float frequency;
	int enabled;
} animationParameters_ubo;

layout (set = 0, binding = 3) uniform sampler2D windmapSampler;

layout (std140, set = 0, binding = 4) uniform UBO_sphereList 
{
	float numSpheres;
	vec3 padding;
	Sphere spheres[64];
} sphereList_ubo;

layout (push_constant) uniform PushConstants {
	float modelHeight;
} pushConstants;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec3 OutPosW;
layout (location = 2) out vec3 OutNormalW;
layout (location = 3) out vec2 OutTex;
layout (location = 4) out vec3 OutNormalV;
layout (location = 5) out vec2 OutTextureTiling;
layout (location = 6) out mat3 OutTBN;

out gl_PerVertex 
{
	vec4 gl_Position;   
};

vec2 transformToUv(vec2 posW)
{
	vec2 uv = posW;
	uv += animationParameters_ubo.terrainSize / 2.0f; 
	uv /= animationParameters_ubo.terrainSize;

	return uv;
}

void main() 
{
	// Todo: Workaround since glslang reflection removes unused vertex input
	vec3 color = InColor;
	vec3 localPos = InPosL.xyz;

	vec3 T = normalize(mat3(InInstanceWorld) * InTangentL);
	vec3 B = normalize(mat3(InInstanceWorld) * InBitangentL);
	vec3 N = normalize(mat3(InInstanceWorld) * InNormalL);
	OutTBN = mat3(T, B, N);
	OutPosW = (InInstanceWorld * vec4(localPos, 1.0)).xyz;

	OutColor = vec4(1.0);
	OutNormalW  = transpose(inverse(mat3(InInstanceWorld))) * InNormalL;
	mat3 normalMatrix = transpose(inverse(mat3(per_frame_vs.view * InInstanceWorld)));
	OutNormalV = normalMatrix * InNormalL;
	OutTex = InTex;
	OutTextureTiling = vec2(1.0, 1.0);

	// Wind animation
	float modelHeight = pushConstants.modelHeight;
	if (animationParameters_ubo.enabled == 1)
	{
		float time = animationParameters_ubo.time;
		vec2 uv = transformToUv(vec2(OutPosW.x, OutPosW.z));
		uv = fract(uv * 400 + time / animationParameters_ubo.frequency);
		vec3 windDir = texture(windmapSampler, uv).xyz;
		windDir = windDir * 2 - 1.0f; // To [-1, 1] range
		localPos.xyz += (localPos.y / modelHeight) * (localPos.y / modelHeight) * windDir * animationParameters_ubo.strength;
	}

	// Bend vegetation from collision spheres
	for(int i = 0; i < sphereList_ubo.numSpheres; i++)
	{
		vec3 spherePos = -sphereList_ubo.spheres[i].position;
		float radius = sphereList_ubo.spheres[i].radius * 1.5;
		spherePos.y -= sphereList_ubo.spheres[i].radius;
		float distToCenter = distance(spherePos, OutPosW.xyz);
		vec3 dir = normalize(OutPosW.xyz - spherePos);
		dir.yz *= -1; // Note: * -1, Unclear
		float heightFactor = (localPos.y / modelHeight) * (localPos.y / modelHeight);

		if (distance(spherePos, OutPosW.xyz) < radius)
		{
			localPos.xyz -= heightFactor * dir * (1 - distToCenter / radius) * 15.0f;
			localPos.y = max(localPos.y, 10.0f); // The foliage needs to be above the ground
		}
	}

	gl_Position = per_frame_vs.projection * per_frame_vs.view * InInstanceWorld * vec4(localPos, 1.0);
}