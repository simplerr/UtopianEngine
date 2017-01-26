#include "StaticModel.h"
#include "VulkanDebug.h"
#include "Device.h"

namespace VulkanLib
{
	StaticModel::StaticModel()
	{

	}

	StaticModel::~StaticModel()
	{

	}

	void StaticModel::AddMesh(Mesh & mesh)
	{
		mMeshes.push_back(mesh);
	}

	void StaticModel::BuildBuffers(Device* device)
	{
		std::vector<Vertex> vertexVector;
		std::vector<uint32_t> indexVector;

		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);

		// All the vertices & indices from the different meshes needs to be combined into one vector
		for (int meshId = 0; meshId < mMeshes.size(); meshId++)
		{
			for (int i = 0; i < mMeshes[meshId].vertices.size(); i++)
			{
				Vertex vertex = mMeshes[meshId].vertices[i];
				vertexVector.push_back(vertex);

				if (vertex.Pos.x < min.x)
					min.x = vertex.Pos.x;
				else if (vertex.Pos.x > max.x)
					max.x = vertex.Pos.x;

				if (vertex.Pos.y < min.y)
					min.y = vertex.Pos.y;
				else if (vertex.Pos.y > max.y)
					max.y = vertex.Pos.y;

				if (vertex.Pos.z < min.z)
					min.z = vertex.Pos.z;
				else if (vertex.Pos.z > max.z)
					max.z = vertex.Pos.z;
			}

			for (int i = 0; i < mMeshes[meshId].indices.size(); i++)
			{
				indexVector.push_back(mMeshes[meshId].indices[i]);
			}
		}

		mBoundingBox = BoundingBox(min, max);

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

		VulkanDebug::ErrorCheck(vkCreateBuffer(device->GetVkDevice(), &vertexBufferInfo, nullptr, &vertices.buffer));								// Create buffer
		vkGetBufferMemoryRequirements(device->GetVkDevice(), vertices.buffer, &memoryRequirments);												// Get buffer size
		memoryAllocation.allocationSize = memoryRequirments.size;
		device->GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocation.memoryTypeIndex);		// Get memory type
		VulkanDebug::ErrorCheck(vkAllocateMemory(device->GetVkDevice(), &memoryAllocation, nullptr, &vertices.memory));							// Allocate device memory
		VulkanDebug::ErrorCheck(vkMapMemory(device->GetVkDevice(), vertices.memory, 0, memoryAllocation.allocationSize, 0, &data));				// Map device memory so the host can access it through data
		memcpy(data, vertexVector.data(), vertexBufferSize);																						// Copy buffer data to the mapped data pointer
		vkUnmapMemory(device->GetVkDevice(), vertices.memory);																					// Unmap memory
		VulkanDebug::ErrorCheck(vkBindBufferMemory(device->GetVkDevice(), vertices.buffer, vertices.memory, 0));									// Bind the buffer to the allocated device memory

		//
		// Create the index buffer
		//
		VkBufferCreateInfo indexBufferInfo = {};
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferInfo.size = indexBufferSize;

		memset(&indices, 0, sizeof(indices));
		VulkanDebug::ErrorCheck(vkCreateBuffer(device->GetVkDevice(), &indexBufferInfo, nullptr, &indices.buffer));								// Create buffer
		vkGetBufferMemoryRequirements(device->GetVkDevice(), indices.buffer, &memoryRequirments);													// Get buffer size
		memoryAllocation.allocationSize = memoryRequirments.size;
		device->GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocation.memoryTypeIndex);		// Get memory type
		VulkanDebug::ErrorCheck(vkAllocateMemory(device->GetVkDevice(), &memoryAllocation, nullptr, &indices.memory));							// Allocate device memory
		VulkanDebug::ErrorCheck(vkMapMemory(device->GetVkDevice(), indices.memory, 0, memoryAllocation.allocationSize, 0, &data));				// Map device memory so the host can access it through data
		memcpy(data, indexVector.data(), indexBufferSize);																							// Copy buffer data to the mapped data pointer
		vkUnmapMemory(device->GetVkDevice(), indices.memory);																						// Unmap memory
		VulkanDebug::ErrorCheck(vkBindBufferMemory(device->GetVkDevice(), indices.buffer, indices.memory, 0));									// Bind the buffer to the allocated device memory

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

	BoundingBox StaticModel::GetBoundingBox()
	{
		return mBoundingBox;
	}
}	// VulkanLib namespace