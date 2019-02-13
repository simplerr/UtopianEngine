#version 450

layout (vertices = 4) out;
 
layout (location = 0) in vec3 InNormalL[];
layout (location = 1) in vec2 InTex[];
 
layout (location = 0) out vec3 OutNormalL[4];
layout (location = 1) out vec2 OutTex[4];
 
layout (std140, set = 0, binding = 1) uniform UBO_settings
{
	float tessellationFactor;
} ubo;

void main()
{
	if (gl_InvocationID == 0)
	{
        // Tessellation factor can be set to zero by example
        // to demonstrate a simple passthrough
        gl_TessLevelOuter[0] = ubo.tessellationFactor;
        gl_TessLevelOuter[1] = ubo.tessellationFactor;
        gl_TessLevelOuter[2] = ubo.tessellationFactor;
        gl_TessLevelOuter[3] = ubo.tessellationFactor;
        gl_TessLevelInner[0] = ubo.tessellationFactor;
        gl_TessLevelInner[1] = ubo.tessellationFactor;
	}

	gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
	OutNormalL[gl_InvocationID] = InNormalL[gl_InvocationID];
	OutTex[gl_InvocationID] = InTex[gl_InvocationID];
} 
