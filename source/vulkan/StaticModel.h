#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "Collision.h"
#include "Vertex.h"

using namespace glm;

namespace VulkanLib
{
	class Device;

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::string texturePath;
	};


	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		void AddMesh(Mesh& mesh);
		void BuildBuffers(Device* device);		// Gets called in ModelLoader::LoadModel()

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
		BoundingBox GetBoundingBox();

		std::vector<Mesh> mMeshes;
	private:
		uint32_t mIndicesCount;
		uint32_t mVerticesCount;

		BoundingBox mBoundingBox;
	};
}	// VulkanLib namespace