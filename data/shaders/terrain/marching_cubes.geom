#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (points) in;
layout (points, max_vertices = 8) out;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	vec4 offsets[8];
	vec4 color;
	float voxelSize;
	float time;
} ubo;

layout (location = 0) out vec3 outColor;

layout(push_constant) uniform PushConsts {
	 mat4 world;		
	 mat4 worldInvTranspose;
} pushConsts;

float sdSphere(vec3 p, float s)
{
  return length(p)-s;
}

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

void CreatePoint(vec3 pos, float size, vec3 color)
{
	gl_Position = ubo.projection * ubo.view * pushConsts.world * vec4(pos, 1.0);
	gl_PointSize = size;
	outColor = color;
	EmitVertex();
}

float density(vec3 pos)
{
	float density = 1;	

	vec2 t = vec2(1000, 500);
	density = sdTorus(pos, t); 
	density = sdSphere(pos, 2500 * abs(sin(ubo.time)));

	return density;
}

void main(void)
{	
	vec3 red = vec3(1, 0, 0);
	vec3 green = vec3(0, 1, 0);
	vec3 blue = vec3(0, 0, 1);
	vec3 black = vec3(0, 0, 0);
	vec3 white = vec3(1, 1, 1);

	for(int i=0; i<gl_in.length(); i++)
	{
		vec3 pos = gl_in[i].gl_Position.xyz;
		float isoLevel = 0.0f;

		int cubeIndex = 0;
		for(int i = 0; i < 8; i++)
		{
			if(density(pos + ubo.offsets[i].xyz) < isoLevel)
				cubeIndex |= (1 << i);
		}

		if(cubeIndex != 0 && cubeIndex != 0xff)
		{
			for(int i = 0; i < 8; i++)
			{
				if(density(pos + ubo.offsets[i].xyz) < isoLevel)
					CreatePoint(pos + ubo.offsets[i].xyz, 5, ubo.color.xyz);
				else if(density(pos + ubo.offsets[i].xyz) > isoLevel)
					CreatePoint(pos + ubo.offsets[i].xyz, 5, ubo.color.xyz);
			}
		}

		EndPrimitive();
	}
}