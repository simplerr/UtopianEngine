#include "Block.h"
#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/handles/DescriptorSet.h"

Block::Block(Vulkan::Renderer* renderer, glm::vec3 position, uint32_t blockSize, float voxelSize, Vulkan::DescriptorSetLayout* desscriptorSetLayout, Vulkan::DescriptorPool* descriptorPool)
{
	mPosition = position;
	mGenerated = false;
	mModified = false;
	mVoxelSize = voxelSize;

	/*
		Storage buffer test
	*/
	mCounterSSBO.numVertices = 0;

	mCounterSSBO.Create(renderer->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	mCounterSSBO.UpdateMemory(renderer->GetVkDevice());


	for (uint32_t i = 0; i < blockSize*blockSize*blockSize*5*3; i++)
	{
		mVertexSSBO.vertices.push_back(GeometryVertex(glm::vec4(i, i, i, i), glm::vec4(-1, -1, -1, -1)));
	}

	mVertexSSBO.Create(renderer->GetDevice(),
						 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
						 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	mVertexSSBO.UpdateMemory(renderer->GetVkDevice());


	void* mappedData;
	mVertexSSBO.MapMemory(0, mVertexSSBO.vertices.size() * sizeof(GeometryVertex), 0, &mappedData);

	GeometryVertex* data = (GeometryVertex*)mappedData;
	for (uint32_t i = 0; i < mVertexSSBO.vertices.size(); i++)
	{
		GeometryVertex d = *data;
		if (d.normal != glm::vec4(-1, -1, -1, -1))
			Vulkan::VulkanDebug::ConsolePrint(d.pos, "Mapped output buffer");

		data++;
	}

	mVertexSSBO.UnmapMemory();

	mDescriptorSet = new Vulkan::DescriptorSet(renderer->GetDevice(), desscriptorSetLayout, descriptorPool);
	mDescriptorSet->AllocateDescriptorSets();
	mDescriptorSet->BindStorageBuffer(0, &mVertexSSBO.GetDescriptor());
	mDescriptorSet->BindStorageBuffer(1, &mCounterSSBO.GetDescriptor());
	mDescriptorSet->UpdateDescriptorSets();
}

Block::~Block()
{
	delete mVertexBuffer;
}

Vulkan::Buffer* Block::GetVertexBuffer()
{
	return mVertexBuffer;
}

bool Block::IsGenerated()
{
	return mGenerated;
}

bool Block::IsModified()
{
	return mModified;
}

void Block::SetGenerated(bool generated)
{
	mGenerated = generated;
}

void Block::SetModified(bool modified)
{
	mModified = modified;
}

void Block::SetNumVertices(uint32_t numVertices)
{
	mNumVertices = numVertices;
}

glm::vec3 Block::GetPosition()
{
	return mPosition;
}

Vulkan::DescriptorSet* Block::GetDescriptorSet()
{
	return mDescriptorSet;
}

uint32_t Block::GetNumVertices()
{
	return mNumVertices;
}

