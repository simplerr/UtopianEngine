#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

layout (location = 0) in float InSize[];
layout (location = 1) in vec4 InColor[];

layout (location = 0) out float OutSize;
layout (location = 1) out vec4 OutColor;
layout (location = 2) out float OutEdgeDistance;

void main(void)
{
    vec2 pos0 = gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
    vec2 pos1 = gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
    
    vec2 dir = pos0 - pos1;
    dir = normalize(vec2(dir.x, dir.y * sharedVariables.viewportSize.y / sharedVariables.viewportSize.x));
    vec2 tng0 = vec2(-dir.y, dir.x);
    vec2 tng1 = tng0 * InSize[1] / sharedVariables.viewportSize;
    tng0 = tng0 * InSize[0] / sharedVariables.viewportSize;
		
	// Line start
    gl_Position = vec4((pos0 - tng0) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw); 
    OutEdgeDistance = -InSize[0];
    OutSize = InSize[0];
    OutColor = InColor[0];
    EmitVertex();
    
    gl_Position = vec4((pos0 + tng0) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw);
    OutEdgeDistance = InSize[0];
    OutSize = InSize[0];
    OutColor = InColor[0];
    EmitVertex();
		
	// Line end
    gl_Position = vec4((pos1 - tng1) * gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
    OutEdgeDistance = -InSize[1];
    OutSize = InSize[1];
    OutColor = InColor[1];
    EmitVertex();
    
    gl_Position = vec4((pos1 + tng1) * gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
    OutEdgeDistance = InSize[1];
    OutSize = InSize[1];
    OutColor = InColor[1];
    EmitVertex();

    EndPrimitive();
}