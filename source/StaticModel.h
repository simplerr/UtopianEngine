#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan\vulkan.h>
#include "../base/vulkanTextureLoader.hpp"

using namespace glm;

namespace VulkanLib
{
	class VulkanBase;

	struct Vertex
	{
		Vertex() {}
		Vertex(vec3 pos) : Pos(pos) {}
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

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
	};

	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		void AddMesh(Mesh& mesh);
		void BuildBuffers(VulkanBase* vulkanBase);		// Gets called in ModelLoader::LoadModel()

		struct {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertices;

		struct {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} indices;

		int GetNumIndices();
		int GetNumVertics();

		vkTools::VulkanTexture* texture;

		std::vector<Mesh> mMeshes;
	private:

		uint32_t mIndicesCount;
		uint32_t mVerticesCount;
	};
}	// VulkanLib namespace