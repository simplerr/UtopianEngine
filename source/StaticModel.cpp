#include "StaticModel.h"
#include "VulkanDebug.h"
#include "VulkanBase.h"

namespace VulkanLib
{
	StaticModel::StaticModel()
	{
		texture = nullptr;
	}

	StaticModel::~StaticModel()
	{
		// The model loader destroys the buffers for us
		delete texture;
	}

	void StaticModel::AddMesh(Mesh & mesh)
	{
		mMeshes.push_back(mesh);
	}

	void StaticModel::BuildBuffers(VulkanBase* vulkanBase)
	{
		std::vector<Vertex> vertexVector;
		std::vector<uint32_t> indexVector;

		// All the vertices & indices from the different meshes needs to be combined into one vector
		for (int meshId = 0; meshId < mMeshes.size(); meshId++)
		{
			for (int i = 0; i < mMeshes[meshId].vertices.size(); i++)
				vertexVector.push_back(mMeshes[meshId].vertices[i]);

			for (int i = 0; i < mMeshes[meshId].indices.size(); i++)
				indexVector.push_back(mMeshes[meshId].indices[i]);
		}

		uint32_t vertexBufferSize = vertexVector.size() * sizeof(Vertex);
		uint32_t indexBufferSize = indexVector.size() * sizeof(uint32_t);

		mIndicesCount = indexVector.size();	// NOTE maybe not smart
		mVerticesCount = vertexVector.size();

		VkMemoryRequirements memoryRequirments;
		VkMemoryAllocateInfo memoryAllocation = {};
		memoryAllocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		void *data;				// Used for memcpy

		//
		// Create the vertex buffer
		//
		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferInfo.size = vertexBufferSize;

		VulkanDebug::ErrorCheck(vkCreateBuffer(vulkanBase->GetDevice(), &vertexBufferInfo, nullptr, &vertices.buffer));								// Create buffer
		vkGetBufferMemoryRequirements(vulkanBase->GetDevice(), vertices.buffer, &memoryRequirments);												// Get buffer size
		memoryAllocation.allocationSize = memoryRequirments.size;
		vulkanBase->GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocation.memoryTypeIndex);		// Get memory type
		VulkanDebug::ErrorCheck(vkAllocateMemory(vulkanBase->GetDevice(), &memoryAllocation, nullptr, &vertices.memory));							// Allocate device memory
		VulkanDebug::ErrorCheck(vkMapMemory(vulkanBase->GetDevice(), vertices.memory, 0, memoryAllocation.allocationSize, 0, &data));				// Map device memory so the host can access it through data
		memcpy(data, vertexVector.data(), vertexBufferSize);																						// Copy buffer data to the mapped data pointer
		vkUnmapMemory(vulkanBase->GetDevice(), vertices.memory);																					// Unmap memory
		VulkanDebug::ErrorCheck(vkBindBufferMemory(vulkanBase->GetDevice(), vertices.buffer, vertices.memory, 0));									// Bind the buffer to the allocated device memory

		//
		// Create the index buffer
		//
		VkBufferCreateInfo indexBufferInfo = {};
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferInfo.size = indexBufferSize;

		memset(&indices, 0, sizeof(indices));
		VulkanDebug::ErrorCheck(vkCreateBuffer(vulkanBase->GetDevice(), &indexBufferInfo, nullptr, &indices.buffer));								// Create buffer
		vkGetBufferMemoryRequirements(vulkanBase->GetDevice(), indices.buffer, &memoryRequirments);													// Get buffer size
		memoryAllocation.allocationSize = memoryRequirments.size;
		vulkanBase->GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocation.memoryTypeIndex);		// Get memory type
		VulkanDebug::ErrorCheck(vkAllocateMemory(vulkanBase->GetDevice(), &memoryAllocation, nullptr, &indices.memory));							// Allocate device memory
		VulkanDebug::ErrorCheck(vkMapMemory(vulkanBase->GetDevice(), indices.memory, 0, memoryAllocation.allocationSize, 0, &data));				// Map device memory so the host can access it through data
		memcpy(data, indexVector.data(), indexBufferSize);																							// Copy buffer data to the mapped data pointer
		vkUnmapMemory(vulkanBase->GetDevice(), indices.memory);																						// Unmap memory
		VulkanDebug::ErrorCheck(vkBindBufferMemory(vulkanBase->GetDevice(), indices.buffer, indices.memory, 0));									// Bind the buffer to the allocated device memory

		// TODO:
		// The mMeshes vector with all the vertices and indices can now actually be destroyed, no need for it any more
	}

	int StaticModel::GetNumIndices()
	{
		return mIndicesCount;
	}

	int StaticModel::GetNumVertics()
	{
		return mVerticesCount;
	}
}	// VulkanLib namespace