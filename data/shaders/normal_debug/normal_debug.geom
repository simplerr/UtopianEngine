#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (set = 0, binding = 0) uniform UBO_viewProjection 
{
	mat4 projection;
	mat4 view;
} ubo;

layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec3 outColor;

layout(push_constant) uniform PushConsts {
	 mat4 world;
	 mat4 worldInvTranspose;
	 vec4 color;
} pushConsts;

void main(void)
{	
	float normalLength = 0.5;
	for(int i=0; i<gl_in.length(); i++)
	{
		vec3 pos = gl_in[i].gl_Position.xyz;
		vec3 normal = normalize(inNormal[i].xyz);

		gl_Position = ubo.projection * ubo.view * pushConsts.world * vec4(pos, 1.0);
		gl_PointSize = 1;
		outColor = vec3(1.0, 0.0, 0.0);
		EmitVertex();

		// Todo: Correct this calculation so that the scaling does not affect normal length
		gl_Position = ubo.projection * ubo.view * pushConsts.world * vec4(pos + normal * normalLength, 1.0);

		outColor = vec3(0.0, 0.0, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}