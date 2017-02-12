#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "Mesh.h"

namespace VulkanLib
{
	Mesh::Mesh(Device* device)
	{
		mDevice = device;
	}
	
	Mesh::~Mesh()
	{
		// Free vertex and index buffers
		vkDestroyBuffer(mDevice->GetVkDevice(), vertices.buffer, nullptr);
		vkFreeMemory(mDevice->GetVkDevice(), vertices.memory, nullptr);
		vkDestroyBuffer(mDevice->GetVkDevice(), indices.buffer, nullptr);
		vkFreeMemory(mDevice->GetVkDevice(), indices.memory, nullptr);
	}

	void Mesh::AddVertex(Vertex vertex)
	{
		vertexVector.push_back(vertex);
	}

	void Mesh::AddVertex(float x, float y, float z)
	{
		AddVertex(Vertex(x, y, z));
	}

	void Mesh::AddIndex(uint32_t v1, uint32_t v2, uint32_t v3)
	{
		indexVector.push_back(v1);
		indexVector.push_back(v2);
		indexVector.push_back(v3);
	}

	void Mesh::BuildBuffers(Device* device)
	{
		mVerticesCount = vertexVector.size();
		mIndicesCount = indexVector.size();

		uint32_t vertexBufferSize = mVerticesCount * sizeof(Vertex);
		uint32_t indexBufferSize = mIndicesCount * sizeof(uint32_t);

		VkMemoryRequirements memoryRequirments;
		VkMemoryAllocateInfo memoryAllocation = {};
		memoryAllocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		void* data;				

		//
		// Create the vertex buffer
		//
		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferInfo.size = vertexBufferSize;

		VulkanDebug::ErrorCheck(vkCreateBuffer(device->GetVkDevice(), &vertexBufferInfo, nullptr, &vertices.buffer));							// Create buffer
		vkGetBufferMemoryRequirements(device->GetVkDevice(), vertices.buffer, &memoryRequirments);												// Get buffer size
		memoryAllocation.allocationSize = memoryRequirments.size;
		device->GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocation.memoryTypeIndex);		// Get memory type
		VulkanDebug::ErrorCheck(vkAllocateMemory(device->GetVkDevice(), &memoryAllocation, nullptr, &vertices.memory));							// Allocate device memory
		VulkanDebug::ErrorCheck(vkMapMemory(device->GetVkDevice(), vertices.memory, 0, memoryAllocation.allocationSize, 0, &data));				// Map device memory so the host can access it through data
		memcpy(data, vertexVector.data(), vertexBufferSize);																						// Copy buffer data to the mapped data pointer
		vkUnmapMemory(device->GetVkDevice(), vertices.memory);																					// Unmap memory
		VulkanDebug::ErrorCheck(vkBindBufferMemory(device->GetVkDevice(), vertices.buffer, vertices.memory, 0));								// Bind the buffer to the allocated device memory

		//
		// Create the index buffer
		//
		VkBufferCreateInfo indexBufferInfo = {};
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferInfo.size = indexBufferSize;

		memset(&indices, 0, sizeof(indices));
		VulkanDebug::ErrorCheck(vkCreateBuffer(device->GetVkDevice(), &indexBufferInfo, nullptr, &indices.buffer));								// Create buffer
		vkGetBufferMemoryRequirements(device->GetVkDevice(), indices.buffer, &memoryRequirments);												// Get buffer size
		memoryAllocation.allocationSize = memoryRequirments.size;
		device->GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocation.memoryTypeIndex);		// Get memory type
		VulkanDebug::ErrorCheck(vkAllocateMemory(device->GetVkDevice(), &memoryAllocation, nullptr, &indices.memory));							// Allocate device memory
		VulkanDebug::ErrorCheck(vkMapMemory(device->GetVkDevice(), indices.memory, 0, memoryAllocation.allocationSize, 0, &data));				// Map device memory so the host can access it through data
		memcpy(data, indexVector.data(), indexBufferSize);																							// Copy buffer data to the mapped data pointer
		vkUnmapMemory(device->GetVkDevice(), indices.memory);																					// Unmap memory
		VulkanDebug::ErrorCheck(vkBindBufferMemory(device->GetVkDevice(), indices.buffer, indices.memory, 0));									// Bind the buffer to the allocated device memory
	}

	void Mesh::BuildBuffers(const std::vector<Vertex>& vertices, std::vector<uint32_t>)
	{

	}

	BoundingBox Mesh::GetBoundingBox()
	{
		return mBoundingBox;
	}

	uint32_t Mesh::GetNumIndices()
	{
		return mIndicesCount;
	}

	void Mesh::SetTexturePath(std::string texturePath)
	{
		mTexturePath = texturePath;
	}
}
