#version 450

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

// Instancing input
layout (location = 6) in mat4 InInstanceWorld;

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
} animationParameters_ubo;

layout (set = 0, binding = 3) uniform sampler2D windmapSampler;

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

	// Wind animation
	float timeScale = 10000.0f;
	float meshHeight = 8.0;
	float time = animationParameters_ubo.time;
	vec2 uv = transformToUv(vec2(OutPosW.x, OutPosW.z));
	uv = fract(uv * 400 + time / timeScale);
	vec2 windDir = texture(windmapSampler, uv).xz;
	windDir = windDir * 2 - 1.0f; // To [-1, 1] range
	localPos.xz += (localPos.y / meshHeight) * (localPos.y / meshHeight) * windDir * animationParameters_ubo.strength;

	OutColor = vec4(1.0);
	OutNormalW  = transpose(inverse(mat3(InInstanceWorld))) * InNormalL;
	mat3 normalMatrix = transpose(inverse(mat3(per_frame_vs.view * InInstanceWorld)));
	OutNormalV = normalMatrix * InNormalL;
	OutTex = InTex;
	OutTextureTiling = vec2(1.0, 1.0);

	gl_Position = per_frame_vs.projection * per_frame_vs.view * InInstanceWorld * vec4(localPos, 1.0);
}