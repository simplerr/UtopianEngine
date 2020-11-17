#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VertexDescription.h"

namespace Utopian::Vk
{
	struct Vertex
	{
		Vertex() {
			Pos = glm::vec3(0.0f);
			Color = glm::vec3(1.0f);
			Normal = glm::vec3(0.0f);
			Tex = glm::vec2(0.0f);
			Tangent = glm::vec3(0.0f);
			Bitangent = glm::vec3(0.0f);
		}

		Vertex(glm::vec3 pos) : Pos(pos) {}
		Vertex(float px, float py, float pz) : Pos(glm::vec3(px, py, pz)) {}
		Vertex(float px, float py, float pz, float nx, float ny, float nz)
			: Pos(px, py, pz), Normal(nx, ny, nz) {}
		Vertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty, float tz, float bx, float by, float bz, float u, float v, float r, float g, float b)
			: Pos(px, py, pz), Normal(nx, ny, nz), Tangent(tx, ty, tz), Bitangent(bx, by, bz), Tex(u, v), Color(r, g, b) {}

		Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 tex, glm::vec3 tangent = glm::vec3(0.0f),
			   glm::vec3 bitangent = glm::vec3(0.0f), glm::vec3 color = glm::vec3(1.0f))
			: Pos(position), Normal(normal), Tex(tex), Tangent(tangent), Bitangent(bitangent), Color(color) {}

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
			description.AddAttribute(BINDING_0, Vec3Attribute());	// Location 4 : Tangent
			description.AddAttribute(BINDING_0, Vec3Attribute());	// Location 5 : Bitangent
			return description;
		}

		glm::vec3 Pos;
		glm::vec3 Color;
		glm::vec3 Normal;
		glm::vec2 Tex;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
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
