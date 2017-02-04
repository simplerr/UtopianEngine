#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/Vertex.h"
#include "Collision.h"

namespace VulkanLib
{
	class Device;

	class CubeMesh
	{
	public:
		struct {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertices;

		struct {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} indices;

		CubeMesh(Device* device);
		~CubeMesh();
		void AddVertex(Vertex vertex);
		void AddVertex(float x, float y, float z);
		void AddIndex(uint32_t v1, uint32_t v2, uint32_t v3);
		void BuildBuffers(Device* device);

		BoundingBox GetBoundingBox();

		uint32_t GetNumIndices();

	private:
		Device* mDevice;
		std::vector<Vertex> mVertices;
		std::vector<unsigned int> mIndices;

		BoundingBox mBoundingBox;

		uint32_t mIndicesCount;
		uint32_t mVerticesCount;
	};
}
