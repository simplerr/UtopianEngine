#version 450

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (location = 0) out vec3 OutNormalL;
layout (location = 1) out vec2 OutTex;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	OutNormalL = InNormalL;
    OutTex = InTex;
    gl_Position = vec4(InPosL.xyz, 1.0);

	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InColor;
	vec3 temp2 = InTangentL;
	temp2 = InBitangentL;
}