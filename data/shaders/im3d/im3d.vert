#version 450

layout (location = 0) in vec4 InPositionSize;
layout (location = 1) in vec4 InColor;

layout (location = 0) out float OutSize;
layout (location = 1) out vec4 OutColor;
layout (location = 2) out float OutEdgeDistance;

out gl_PerVertex 
{
	vec4 gl_Position;   
	float gl_PointSize;
};

layout (std140, set = 0, binding = 0) uniform UBO_viewProjection 
{
	mat4 projection;
	mat4 view;
} per_frame_vs;

void main() 
{
    OutSize = InPositionSize.w;
	OutColor = InColor.abgr; // Swizzle for correct endianess
	OutEdgeDistance = 0.0f; // Unused, calculated in GS

	vec3 pos = -InPositionSize.xyz; // Todo: Note the -1
	gl_Position = per_frame_vs.projection * per_frame_vs.view * vec4(pos, 1.0);
	gl_PointSize = InPositionSize.w;
}