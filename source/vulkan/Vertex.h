#pragma once

#include <glm/glm.hpp>

using namespace glm;

namespace VulkanLib
{
	struct Vertex
	{
		Vertex() {}
		Vertex(vec3 pos) : Pos(pos) {}
		Vertex(float px, float py, float pz) : Pos(vec3(px, py, pz)) {}
		Vertex(float px, float py, float pz, float nx, float ny, float nz)
			: Pos(px, py, pz), Normal(nx, ny, nz) {}
		Vertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty, float tz, float u, float v, float r, float g, float b)
			: Pos(px, py, pz), Normal(nx, ny, nz), Tangent(tx, ty, tz, 1.0f), Tex(u, v), Color(r, g, b) {}

		Vertex(vec3 position, vec3 normal, vec2 tex, vec3 tangent, vec3 color)
			: Pos(position), Normal(normal), Tex(tex), Tangent(tangent, 1.0f), Color(color) {}

		vec3 Pos;
		vec3 Color;
		vec3 Normal;
		vec2 Tex;
		vec4 Tangent;
	};
}
