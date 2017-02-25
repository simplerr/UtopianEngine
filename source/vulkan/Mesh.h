#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/Vertex.h"
#include "Collision.h"

namespace VulkanLib
{
	class Device;
	class VulkanTexture;

	class Mesh
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

		Mesh(Device* device);
		~Mesh();

		void AddVertex(Vertex vertex);
		void AddVertex(float x, float y, float z);
		void AddIndex(uint32_t v1, uint32_t v2, uint32_t v3);
		void BuildBuffers(Device* device);
		void BuildBuffers(const std::vector<Vertex>& vertices, std::vector<uint32_t>);

		void SetTexture(VulkanTexture* texture);
		VkDescriptorSet GetTextureDescriptor();

		BoundingBox GetBoundingBox();

		uint32_t GetNumIndices();
		void SetTexturePath(std::string texturePath);

		std::vector<Vertex> vertexVector;
		std::vector<unsigned int> indexVector;
	private:
		Device* mDevice;
		VulkanTexture* mTexture;
		std::string mTexturePath;

		BoundingBox mBoundingBox;

		uint32_t mIndicesCount;
		uint32_t mVerticesCount;
	};
}
