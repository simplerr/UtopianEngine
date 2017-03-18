#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"

namespace Vulkan
{
	class Renderer;
	class Buffer;
	class DescriptorSetLayout;
	class DescriptorSet;
	class DescriptorPool;
}

// Storage buffer test
struct GeometryVertex
{
	GeometryVertex(glm::vec4 _pos, glm::vec4 _normal)
		: pos(_pos), normal(_normal) {

	}

	glm::vec4 pos;
	glm::vec4 normal;
};
	
class VertexSSBO : public Vulkan::ShaderBuffer
{
public:
	virtual void UpdateMemory(VkDevice device)
	{
		uint8_t *mapped;
		uint32_t offset = 0;
		uint32_t size = vertices.size() * sizeof(GeometryVertex);
		mBuffer->MapMemory(offset, size, 0, (void**)&mapped);
		memcpy(mapped, vertices.data(), size);
		mBuffer->UnmapMemory();
	}

	virtual int GetSize()
	{
		return vertices.size() * sizeof(GeometryVertex);
	}

	std::vector<GeometryVertex> vertices;
};

class CounterSSBO : public Vulkan::ShaderBuffer
{
public:
	virtual void UpdateMemory(VkDevice device)
	{
		// Map vertex counter
		uint8_t *mapped;
		uint32_t offset = 0;
		uint32_t size = sizeof(numVertices);
		mBuffer->MapMemory(offset, size, 0, (void**)&mapped);
		memcpy(mapped, &numVertices, size);
		mBuffer->UnmapMemory();
	}

	virtual int GetSize()
	{
		return sizeof(numVertices);
	}

	uint32_t numVertices;
};

class Block
{
public:
	Block(Vulkan::Renderer* renderer, glm::vec3 position, uint32_t blockSize, float voxelSize, Vulkan::DescriptorSetLayout* desscriptorSetLayout, Vulkan::DescriptorPool* descriptorPool);
	~Block();

	Vulkan::Buffer* GetVertexBuffer();

	/* True if a vertex buffer has been generated for this block */
	bool IsGenerated();

	/* True if the block has been modified after the last generation */
	bool IsModified();

	void SetGenerated(bool generated);
	void SetModified(bool modified);
	void SetNumVertices(uint32_t numVertices);

	glm::vec3 GetPosition();
	Vulkan::DescriptorSet* GetDescriptorSet();
	uint32_t GetNumVertices();

	VertexSSBO mVertexSSBO;
	CounterSSBO mCounterSSBO;

private:
	Vulkan::Buffer* mVertexBuffer;
	Vulkan::DescriptorSet* mDescriptorSet;
	glm::vec3 mPosition;

	bool mGenerated;
	bool mModified;
	uint32_t mNumVertices;

	float mVoxelSize;
};
