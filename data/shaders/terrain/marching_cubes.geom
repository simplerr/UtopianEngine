#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (points) in;
layout (points, max_vertices = 6) out;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
} ubo;

layout (location = 0) out vec3 outColor;

layout(push_constant) uniform PushConsts {
	 mat4 world;		
	 mat4 worldInvTranspose;
} pushConsts;

float sdTorus(vec3 p, vec2 t)
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdCone(vec3 p, vec2 c)
{
    // c must be normalized
    float q = length(p.xy);
    return dot(c,vec2(q,p.z));
}

void main(void)
{	
	float normalLength = 1.0;
	for(int i=0; i<gl_in.length(); i++)
	{
		vec3 pos = gl_in[i].gl_Position.xyz;

		vec2 t = vec2(1000, 500);
		//vec2 c = normalize(vec2(50, 30));
		//if(sdCone(pos, c) < 0)
		if(sdTorus(pos, t) < 0)
		{
			vec3 normal = vec3(0, 1, 0);

			gl_Position = ubo.projection * ubo.view * pushConsts.world * vec4(pos, 1.0);
			gl_PointSize = 3;
			outColor = vec3(1.0, 0.0, 0.0);
			EmitVertex();

			//gl_Position = ubo.projection * ubo.view * pushConsts.world * vec4(pos + normal * normalLength, 1.0);
			//outColor = vec3(0.0, 0.0, 1.0);
			//EmitVertex();

			EndPrimitive();
		}
	}
}