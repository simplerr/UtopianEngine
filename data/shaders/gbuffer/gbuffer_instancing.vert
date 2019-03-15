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

void main() 
{
	// Todo: Workaround since glslang reflection removes unused vertex input
	vec3 color = InColor;

	vec3 T = normalize(mat3(InInstanceWorld) * InTangentL);
	vec3 B = normalize(mat3(InInstanceWorld) * InBitangentL);
	vec3 N = normalize(mat3(InInstanceWorld) * InNormalL);
	OutTBN = mat3(T, B, N); // = transpose(mat3(T, B, N));

	OutColor = vec4(1.0);
	OutPosW = (InInstanceWorld * vec4(InPosL.xyz, 1.0)).xyz;
	OutNormalW  = transpose(inverse(mat3(InInstanceWorld))) * InNormalL;
	mat3 normalMatrix = transpose(inverse(mat3(per_frame_vs.view * InInstanceWorld)));
	OutNormalV = normalMatrix * InNormalL;
	OutTex = InTex;
	OutTextureTiling = vec2(1.0, 1.0);

	gl_Position = per_frame_vs.projection * per_frame_vs.view * InInstanceWorld * vec4(InPosL.xyz, 1.0);
}