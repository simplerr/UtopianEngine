#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;			// Vertex in local coordinate system
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;		// Normal in local coordinate system
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangent;

layout (std140, binding = 0) uniform UBO 
{
	// Camera 
	mat4 projection;
	mat4 view;
	
	vec4 lightDir;
	vec3 eyePos;

	float t;
	
	// Constans
	bool useInstancing;
	vec3 garbage;
} per_frame;

layout(push_constant) uniform PushConsts {
	 mat4 world;	// Model View Projection
	 vec3 color;	// Color
} pushConsts;

layout (location = 0) out vec3 OutNormalW;		// Normal in world coordinate system
layout (location = 1) out vec3 OutColor;
layout (location = 2) out vec2 OutTex;
layout (location = 3) out vec3 OutEyeDirW;		// Direction to the eye in world coordinate system

void main() 
{
	OutColor = pushConsts.color; 

	vec3 pos = InPosL;

	OutColor = vec3(1, 1, 1);

	OutTex = InTex;

	gl_Position = per_frame.projection * per_frame.view * pushConsts.world * vec4(pos.xyz, 1.0);
	
    vec4 PosW = pushConsts.world  * vec4(pos, 1.0);
    OutNormalW = mat3(pushConsts.world ) * InNormalL;
    OutEyeDirW = per_frame.eyePos - PosW.xyz;	
}