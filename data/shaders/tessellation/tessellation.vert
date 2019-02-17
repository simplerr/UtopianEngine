#version 450

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (location = 0) out vec3 OutNormalL;
layout (location = 1) out vec2 OutTex;

// Todo: This should be shared between the vertex and the tessellation evaluation shader
layout (set = 0, binding = 4) uniform sampler2D samplerHeightmapVS;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	OutNormalL = InNormalL;
    OutTex = InTex;
    gl_Position = vec4(InPosL.xyz, 1.0);

    // Need to displace the Y coordinate here so that the tessellation factor
    // calculation in the .tesc shader works as expected. Otherwise all vertices will
    // have y=0.0.
    gl_Position.y = texture(samplerHeightmapVS, OutTex).r * 18000.0f;

	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InColor;
	vec3 temp2 = InTangentL;
	temp2 = InBitangentL;
}