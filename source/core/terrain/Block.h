#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"

namespace Utopian::Vk
{
	class VulkanApp;
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
	
class VertexSSBO : public Utopian::Vk::ShaderBuffer
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

class Block
{
public:
	Block(Utopian::Vk::Device* device, glm::vec3 position, glm::vec3 color, uint32_t blockSize, float voxelSize, const Utopian::Vk::DescriptorSetLayout* desscriptorSetLayout, Utopian::Vk::DescriptorPool* descriptorPool);
	~Block();

	Utopian::Vk::Buffer* GetVertexBuffer();

	/* True if a vertex buffer has been generated for this block */
	bool IsGenerated();

	/* True if the block has been modified after the last generation */
	bool IsModified();

	bool IsVisible();

	void SetGenerated(bool generated);
	void SetModified(bool modified);
	void SetVisible(bool visible);
	void SetNumVertices(uint32_t numVertices);
	void SetColor(glm::vec3 color);

	glm::vec3 GetPosition();
	glm::vec3 GetColor();
	uint32_t GetNumVertices();
	VkDescriptorBufferInfo GetBufferInfo();

	// TESTING:
	uint32_t pipelineType;

private:
	Utopian::Vk::Buffer* mVertexBuffer;
	VkDescriptorBufferInfo mBufferInfo;
	glm::vec3 mPosition;
	glm::vec3 mColor;

	bool mGenerated;
	bool mModified;
	bool mVisible;
	uint32_t mNumVertices;

	float mVoxelSize;
};
