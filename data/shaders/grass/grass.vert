#version 450

#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"

/*
	This shader does not need any per-vertex inputs. It will be invoked 4 times per instance so we can use this
	to index into vertexUVPos[] to generate vertex UV and position depending on gl_VertexIndex.
	See: https://www.gamedev.net/forums/topic/662251-questions-about-billboards/

	vertexID | instanceID
	----+----
     0  |   0
     1  |   0
     2  |   0
     3  |   0
     0  |   1
     1  |   1
     2  |   1
	 ...
*/

// Instance data
layout (location = 0) in vec4 InInstancePosW;
layout (location = 1) in vec3 InColor;
layout (location = 2) in int InTexId;

layout (std140, set = 0, binding = 0) uniform UBO_grassSettings
{
	float viewDistance;
} grassSettings_ubo;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec2 OutTex;
layout (location = 2) out float OutEyeDist;
layout (location = 3) out float OutViewDistance; // Todo: Move to separate uniform buffer
layout (location = 4) flat out int OutTexId;

out gl_PerVertex 
{
	vec4 gl_Position;
};

// Represents the vertex positions for our triangle strips
// xy is UV and zw is position
const vec4 vertexUVPos[4] =
{
    { 0.0, 0.0, -0.5, -0.5 },
    { 0.0, 1.0, -0.5, +0.5 },
    { 1.0, 0.0, +0.5, -0.5 },
    { 1.0, 1.0, +0.5, +0.5 },
};

vec4 ComputePosition(vec3 instancePos, float size, vec2 vertexPos)
{
    // Create billboard (quad always facing the camera)
	// Note: Todo: The + sign. This is due to the coordinate system is negated and wrong.
    vec3 toEye = normalize(sharedVariables.eyePos.xyz + instancePos);
    vec3 up    = vec3(0.0f, 1.0f, 0.0f);
    vec3 right = cross(toEye, up);
    //up = cross(toEye, right);
    instancePos += (right * size * vertexPos.x) + (up * size * vertexPos.y);

    return sharedVariables.projectionMatrix * sharedVariables.viewMatrix * vec4(instancePos - size/2.0f, 1.0f);
}

void main() 
{
	OutColor = vec4(1, 1, 1, 1);

	// Todo: correct this
	vec3 instancePos = InInstancePosW.xyz;
	instancePos.xz *= -1;

	float eyeDistance = length(sharedVariables.eyePos.xz + instancePos.xz);
	OutColor.xyz = InColor.xyz;
	OutEyeDist = eyeDistance;
	OutViewDistance = grassSettings_ubo.viewDistance;
	OutTexId = InTexId;

	gl_Position = ComputePosition(instancePos, 40.0f, vertexUVPos[gl_VertexIndex].zw);
	OutTex = vertexUVPos[gl_VertexIndex].xy;
}