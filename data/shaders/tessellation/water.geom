#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_water.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout (location = 0) in vec3 InNormalL[];
layout (location = 1) in vec2 InTex[];
layout (location = 2) in vec3 InPosW[];

layout (location = 0) out vec3 OutNormalL;
layout (location = 1) out vec2 OutTex;
layout (location = 2) out vec3 OutTangent;
layout (location = 3) out vec3 OutPosW;
layout (location = 4) out vec3 OutBarycentric;

vec3 calculateTangent()
{
    vec3 v0 = gl_in[0].gl_Position.xyz;
	vec3 v1 = gl_in[1].gl_Position.xyz;
	vec3 v2 = gl_in[2].gl_Position.xyz;

	// Edges of the face/triangle
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
	
	vec2 uv0 = InTex[0];
	vec2 uv1 = InTex[1];
	vec2 uv2 = InTex[2];

    vec2 deltaUV1 = uv1 - uv0;
	vec2 deltaUV2 = uv2 - uv0;
	
	float r = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	
	vec3 tangent = normalize((e1 * deltaUV2.y - e2 * deltaUV1.y)*r);

    return tangent;
}

void main() 
{
    // The tangent is the same for all vertices since they share triangle face
    vec3 tangent = calculateTangent();

    vec3[3] corners = vec3[](vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));

    // gerstner.glsl alredy calculates normals but this might be used in other water methods
    // vec3 edgeA = InPosW[1] - InPosW[0];
    // vec3 edgeB = InPosW[2] - InPosW[0];
    // OutNormalL = normalize(cross(edgeA, edgeB));

    for (int i = 0; i < gl_in.length(); i++)
    {
        gl_Position = gl_in[i].gl_Position;
        OutNormalL = InNormalL[i];
        OutTex = InTex[i];
        OutPosW = InPosW[i];
        OutTangent = tangent;
        OutBarycentric = corners[i];
        EmitVertex();
    }

    EndPrimitive();
}