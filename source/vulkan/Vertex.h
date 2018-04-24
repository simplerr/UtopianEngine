#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"
#include "vulkan/VertexDescription.h"

using namespace glm;

namespace Utopian::Vk
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

		static VertexDescription GetDescription()
		{
			VertexDescription description;
			description.AddBinding(BINDING_0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

			// We need to tell Vulkan about the memory layout for each attribute
			// 5 attributes: position, normal, texture coordinates, tangent and color
			// See Vertex struct
			description.AddAttribute(BINDING_0, Vec3Attribute());	// Location 0 : Position
			description.AddAttribute(BINDING_0, Vec3Attribute());	// Location 1 : Color
			description.AddAttribute(BINDING_0, Vec3Attribute());	// Location 2 : Normal
			description.AddAttribute(BINDING_0, Vec2Attribute());	// Location 3 : Texture
			description.AddAttribute(BINDING_0, Vec4Attribute());	// Location 4 : Tangent
			return description;
		}

		vec3 Pos;
		vec3 Color;
		vec3 Normal;
		vec2 Tex;
		vec4 Tangent;
	};

	struct ScreenQuadVertex
	{
		static VertexDescription GetDescription()
		{
			VertexDescription description;
			description.AddBinding(BINDING_0, sizeof(ScreenQuadVertex), VK_VERTEX_INPUT_RATE_VERTEX);
			description.AddAttribute(BINDING_0, Utopian::Vk::Vec3Attribute());	// InPosL
			description.AddAttribute(BINDING_0, Utopian::Vk::Vec2Attribute());	// InTex	
			return description;
		}

		glm::vec3 pos;
		glm::vec2 uv;
	};

	struct TerrainVertex
	{
		static VertexDescription GetDescription()
		{
			VertexDescription description;
			description.AddBinding(BINDING_0, sizeof(TerrainVertex), VK_VERTEX_INPUT_RATE_VERTEX);
			description.AddAttribute(BINDING_0, Utopian::Vk::Vec4Attribute());	// InPosL
			description.AddAttribute(BINDING_0, Utopian::Vk::Vec4Attribute());	// InNormal	
			return description;
		}

		glm::vec4 position;
		glm::vec4 normal;
	};

}
