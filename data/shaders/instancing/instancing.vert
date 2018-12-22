#version 450

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

layout (std140, set = 0, binding = 0) uniform UBO_viewProjection 
{
	// Camera 
	mat4 projection;
	mat4 view;
	vec4 eyePos;
} per_frame_vs;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec2 OutTex;

out gl_PerVertex 
{
	vec4 gl_Position;
};

// Represents the vertex positions for our triangle strips
// xy is UV and zw is position
const vec4 vertexUVPos[4] =
{
    { 0.0, 0.0, -1.0, -1.0 },
    { 0.0, 1.0, -1.0, +1.0 },
    { 1.0, 0.0, +1.0, -1.0 },
    { 1.0, 1.0, +1.0, +1.0 },
};

vec4 ComputePosition(vec3 instancePos, float size, vec2 vertexPos)
{
    // Create billboard (quad always facing the camera)
	// Note: Todo: The + sign. This is due to the coordinate system is negated and wrong.
    vec3 toEye = normalize(per_frame_vs.eyePos.xyz + instancePos);
    vec3 up    = vec3(0.0f, 1.0f, 0.0f);
    vec3 right = cross(toEye, up);
    //up = cross(toEye, right);
    instancePos += (right * size * vertexPos.x) + (up * size * vertexPos.y);

    return per_frame_vs.projection * per_frame_vs.view * vec4(instancePos - size / 0.95f, 1.0f);
}

void main() 
{
	OutColor = vec4(1, 1, 1, 1);

	// Todo: correct this
	vec3 instancePos = InInstancePosW.xyz;
	instancePos.xz *= -1;

	float eyeDistance = length(per_frame_vs.eyePos.xyz + instancePos);
	//float grassLerp = clamp((-eyeDistance + 2000.0f) / 200.0f, 0.0, 1.0); 
	// OutColor.a = grassLerp;//0.0f;

	OutColor.xyz = InColor.xyz;

	if (eyeDistance > 2000.0f)
	 	OutColor.a = 0.0f;

	gl_Position = ComputePosition(instancePos, 50.0f, vertexUVPos[gl_VertexIndex].zw);
	OutTex = vertexUVPos[gl_VertexIndex].xy;
}