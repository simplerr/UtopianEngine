#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "CubeMesh.h"

namespace VulkanLib
{
	CubeMesh::CubeMesh(Device* device)
	{
		mDevice = device;
	}
	
	CubeMesh::~CubeMesh()
	{
		// Free vertex and index buffers
		vkDestroyBuffer(mDevice->GetVkDevice(), vertices.buffer, nullptr);
		vkFreeMemory(mDevice->GetVkDevice(), vertices.memory, nullptr);
		vkDestroyBuffer(mDevice->GetVkDevice(), indices.buffer, nullptr);
		vkFreeMemory(mDevice->GetVkDevice(), indices.memory, nullptr);
	}

	void CubeMesh::AddVertex(Vertex vertex)
	{
		mVertices.push_back(vertex);
	}

	void CubeMesh::AddVertex(float x, float y, float z)
	{
		AddVertex(Vertex(x, y, z));
	}

	void CubeMesh::AddIndex(uint32_t v1, uint32_t v2, uint32_t v3)
	{
		mIndices.push_back(v1);
		mIndices.push_back(v2);
		mIndices.push_back(v3);
	}

	void CubeMesh::BuildBuffers(Device * device)
	{
		// Front
		AddVertex(-0.5f, -0.5f, 0.5f);
		AddVertex(0.5f, -0.5f, 0.5f);
		AddVertex(0.5f, 0.5f, 0.5f);
		AddVertex(-0.5f, 0.5f, 0.5f);

		// Back
		AddVertex(-0.5f, -0.5f, -0.5f);
		AddVertex(0.5f, -0.5f, -0.5f);
		AddVertex(0.5f, 0.5f, -0.5f);
		AddVertex(-0.5f, 0.5f, -0.5f);

		// Front
		AddIndex(0, 1, 2);
		AddIndex(2, 3, 0);

		// Top
		AddIndex(1, 5, 6);
		AddIndex(6, 2, 1);

		// Back
		AddIndex(7, 6, 5);
		AddIndex(5, 4, 7);

		// Bottom
		AddIndex(4, 0, 3);
		AddIndex(3, 7, 4);

		// Left
		AddIndex(4, 5, 1);
		AddIndex(1, 0, 4);

		// Right
		AddIndex(3, 2, 6);
		AddIndex(6, 7, 3);

		mVerticesCount = mVertices.size();
		mIndicesCount = mIndices.size();	

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
		memcpy(data, mVertices.data(), vertexBufferSize);																						// Copy buffer data to the mapped data pointer
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
		memcpy(data, mIndices.data(), indexBufferSize);																							// Copy buffer data to the mapped data pointer
		vkUnmapMemory(device->GetVkDevice(), indices.memory);																					// Unmap memory
		VulkanDebug::ErrorCheck(vkBindBufferMemory(device->GetVkDevice(), indices.buffer, indices.memory, 0));									// Bind the buffer to the allocated device memory
	}

	BoundingBox CubeMesh::GetBoundingBox()
	{
		return mBoundingBox;
	}

	uint32_t CubeMesh::GetNumIndices()
	{
		return mIndicesCount;
	}
}
