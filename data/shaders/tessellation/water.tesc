#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_water.glsl"
#include "shared_variables.glsl"

layout (vertices = 4) out;
 
layout (location = 0) in vec3 InNormalL[];
layout (location = 1) in vec2 InTex[];
 
layout (location = 0) out vec3 OutNormalL[4];
layout (location = 1) out vec2 OutTex[4];

// Calculate the tessellation factor based on screen space
// dimensions of the edge
// From Sascha Willems examples.
float screenSpaceTessFactor(vec4 p0, vec4 p1)
{
	// Calculate edge mid point
	vec4 midPoint = 0.5 * (p0 + p1);
	// Sphere radius as distance between the control points
	float radius = distance(p0, p1) / 2.0;

	// View space
	vec4 v0 = sharedVariables.viewMatrix * midPoint;

	// Project into clip space
	vec4 clip0 = (sharedVariables.projectionMatrix * (v0 - vec4(radius, vec3(0.0))));
	vec4 clip1 = (sharedVariables.projectionMatrix * (v0 + vec4(radius, vec3(0.0))));

	// Get normalized device coordinates
	clip0 /= clip0.w;
	clip1 /= clip1.w;

	// Convert to viewport coordinates
	clip0.xy *= ubo_settings.viewportSize;
	clip1.xy *= ubo_settings.viewportSize;
	
	// Return the tessellation factor based on the screen size 
	// given by the distance of the two edge control points in screen space
	// and a reference (min.) tessellation size for the edge set by the application
    float edgeSize = 30.0f;
	return clamp(distance(clip0, clip1) / edgeSize * ubo_settings.tessellationFactor, 1.0, 64.0);
}

bool frustumCheck()
{
	// Fixed radius, need to be updated if patch size changes
	const float radius = 220.0f;
	vec4 pos = gl_in[gl_InvocationID].gl_Position;

	// Check sphere against frustum planes
	for (int i = 0; i < 6; i++) {
		if (dot(pos, ubo_frustum.frustumPlanes[i]) + radius < 0.0)
		{
			return false;
		}
	}
	return true;
}

void main()
{
	if (gl_InvocationID == 0)
	{
		if (!frustumCheck())
		{
			// Discard patch
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
		}
		else
		{
			if (ubo_settings.tessellationFactor > 0.0)
			{
				gl_TessLevelOuter[0] = screenSpaceTessFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
				gl_TessLevelOuter[1] = screenSpaceTessFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
				gl_TessLevelOuter[2] = screenSpaceTessFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
				gl_TessLevelOuter[3] = screenSpaceTessFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);
				gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
				gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
			}
			else
			{
				// No tessellation, simple passthrough
				gl_TessLevelOuter[0] = 1.0f;
				gl_TessLevelOuter[1] = 1.0f;
				gl_TessLevelOuter[2] = 1.0f;
				gl_TessLevelOuter[3] = 1.0f;
				gl_TessLevelInner[0] = 1.0f;
				gl_TessLevelInner[1] = 1.0f;
			}
		}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	//gl_out[gl_InvocationID].gl_Position.y = 0.0f;
	OutNormalL[gl_InvocationID] = InNormalL[gl_InvocationID];
	OutTex[gl_InvocationID] = InTex[gl_InvocationID];
} 
