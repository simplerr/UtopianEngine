#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (points) in;
layout (triangle_strip, max_vertices = 15) out;

layout (set = 0, binding = 0) uniform isampler2D edgeTableTex;
layout (set = 0, binding = 2) uniform isampler2D triangleTableTex;

layout (set = 0, binding = 1) uniform UBO 
{
	mat4 projection;
	mat4 view;
	vec4 offsets[8];
	vec4 color;
	float voxelSize;
	float time;
} ubo;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;

struct GeometryVertex
{
	vec4 pos;
	vec4 normal;
};

layout(std430, binding = 3) buffer VertexSSBO 
{
	GeometryVertex vertices[];
} vertexSSBO;

layout(std430, binding = 4) buffer CounterSSBO 
{
	int vertexCount;
} counterSSBO;



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

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float opI( float d1, float d2 )
{
    return max(d1,d2);
}

void CreatePoint(vec3 pos, float size, vec3 color)
{
	gl_Position = ubo.projection * ubo.view * vec4(pos, 1.0);
	gl_PointSize = size;
	outColor = color;
	EmitVertex();
}

float density(vec3 pos)
{
	float density = 1;	

	vec2 t = vec2(700, 500);
	density = sdTorus(pos+1000, t); 
	density = min(-pos.y, density);
	//density = sdBox(pos, vec3(10));
	//return -pos.y;
	//return min(min(density, -pos.y), sdSphere(pos+vec3(4500, 1000, 4500), 500*abs(sin(ubo.time))));
	return min(density, sdSphere(pos+vec3(4500, 1000, 4500), 500*abs(sin(ubo.time))));
	//density = sdSphere(pos, 2500 * abs(sin(ubo.time)));

	return density;
}

float density(int corner)
{
	vec3 pos = (gl_in[0].gl_Position).xyz + ubo.offsets[corner].xyz;
	return density(pos);
}

vec3 vertexInterp(float isolevel, vec3 pos1, vec3 pos2, float val1, float val2)
{
	return mix(pos1, pos2, (isolevel-val1)/(val2-val1));
}

vec3 cornerPos(vec3 pos, int corner)
{
	return pos + ubo.offsets[corner].xyz;
}

vec3 cornerPos(int corner)
{
	return (gl_in[0].gl_Position).xyz + ubo.offsets[corner].xyz;
} 

int edgeTableValue(int cubeIndex)
{
	return texelFetch(edgeTableTex, ivec2(cubeIndex, 0), 0).r;
}

int triangleTableValue(int cubeIndex, int i)
{
	return texelFetch(triangleTableTex, ivec2(i, cubeIndex), 0).r;
}

vec3 generateNormal(vec3 pos)
{
	vec3 grad;
	float d = 1.0 / 32.0;

	grad.x = density(pos + vec3(d, 0, 0)) - density(pos + vec3(-d, 0, 0));
	grad.y = density(pos + vec3(0, d, 0)) - density(pos + vec3(0, -d, 0));
	grad.z = density(pos + vec3(0, 0, d)) - density(pos + vec3(0, 0, -d));

	vec3 normal = -normalize(grad);
	return normal;
}

void main(void)
{	
	outColor = vertexSSBO.vertices[29].pos.xyz;
	outColor = vec3(0, 1, 0);

	if(ubo.time > 1)
		vertexSSBO.vertices[29].pos = vec4(1, 0, 0, 1);

	for(int i=0; i<gl_in.length(); i++)
	{
		vec3 pos = (gl_in[i].gl_Position).xyz;
		float isoLevel = 0.0f;

		int cubeIndex = 0;
		for(int i = 0; i < 8; i++)
		{
			if(density(pos + ubo.offsets[i].xyz) < isoLevel)
				cubeIndex |= (1 << i);
		}

		// No interesction with the isosurface in the cube
		if(texelFetch(edgeTableTex, ivec2(cubeIndex, 0), 0).r != 0)
		{
			vec3 vertList[12];

			if((edgeTableValue(cubeIndex) & 1) != 0)
				vertList[0] = vertexInterp(isoLevel, cornerPos(0), cornerPos(1), density(0), density(1));
			if((edgeTableValue(cubeIndex) & 2) != 0)
				vertList[1] = vertexInterp(isoLevel, cornerPos(1), cornerPos(2), density(1), density(2));
			if((edgeTableValue(cubeIndex) & 4) != 0)
				vertList[2] = vertexInterp(isoLevel, cornerPos(2), cornerPos(3), density(2), density(3));
			if((edgeTableValue(cubeIndex) & 8) != 0)
				vertList[3] = vertexInterp(isoLevel, cornerPos(3), cornerPos(0), density(3), density(0));
			if((edgeTableValue(cubeIndex) & 16) != 0)
				vertList[4] = vertexInterp(isoLevel, cornerPos(4), cornerPos(5), density(4), density(5));
			if((edgeTableValue(cubeIndex) & 32) != 0)
				vertList[5] = vertexInterp(isoLevel, cornerPos(5), cornerPos(6), density(5), density(6));
			if((edgeTableValue(cubeIndex) & 64) != 0)
				vertList[6] = vertexInterp(isoLevel, cornerPos(6), cornerPos(7), density(6), density(7));
			if((edgeTableValue(cubeIndex) & 128) != 0)
				vertList[7] = vertexInterp(isoLevel, cornerPos(7), cornerPos(4), density(7), density(4));
			if((edgeTableValue(cubeIndex) & 256) != 0)
				vertList[8] = vertexInterp(isoLevel, cornerPos(0), cornerPos(4), density(0), density(4));
			if((edgeTableValue(cubeIndex) & 512) != 0)
				vertList[9] = vertexInterp(isoLevel, cornerPos(1), cornerPos(5), density(1), density(5));
			if((edgeTableValue(cubeIndex) & 1024) != 0)
				vertList[10] = vertexInterp(isoLevel, cornerPos(2), cornerPos(6), density(2), density(6));
			if((edgeTableValue(cubeIndex) & 2048) != 0)
				vertList[11] = vertexInterp(isoLevel, cornerPos(3), cornerPos(7), density(3), density(7));


			mat4 viewProjection = ubo.projection * ubo.view;
			vec3 position;
			for(int i = 0; triangleTableValue(cubeIndex, i) != -1; i += 3)
			{
				int index = atomicAdd(counterSSBO.vertexCount, 3);

				position = vertList[triangleTableValue(cubeIndex, i)];
				outNormal = generateNormal(position);
				vertexSSBO.vertices[index].pos = vec4(position, 1);
				vertexSSBO.vertices[index].normal = vec4(outNormal, 1);
				gl_Position = viewProjection * vec4(position, 1.0);
				EmitVertex();

				position = vertList[triangleTableValue(cubeIndex, i + 1)];
				outNormal = generateNormal(position);
				vertexSSBO.vertices[index + 1].pos = vec4(position, 1);
				vertexSSBO.vertices[index + 1].normal = vec4(outNormal, 1);
				gl_Position = viewProjection * vec4(position, 1.0);
				EmitVertex();

				position = vertList[triangleTableValue(cubeIndex, i + 2)];
				outNormal = generateNormal(position);
				vertexSSBO.vertices[index + 2].pos = vec4(position, 1);
				vertexSSBO.vertices[index + 2].normal = vec4(outNormal, 1);
				gl_Position = viewProjection * vec4(position, 1.0);
				EmitVertex();

				EndPrimitive();
			}
		}
	}
}