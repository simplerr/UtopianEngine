#include "Block.h"
#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/handles/DescriptorSet.h"

Block::Block(Vulkan::Renderer* renderer, glm::vec3 position, glm::vec3 color, uint32_t blockSize, float voxelSize, Vulkan::DescriptorSetLayout* desscriptorSetLayout, Vulkan::DescriptorPool* descriptorPool)
{
	mPosition = position;
	mColor = color;
	mGenerated = false;
	mModified = false;
	mVoxelSize = voxelSize;

	/*
		Storage buffer test
	*/
	mCounterSSBO.numVertices = 0;

	mCounterSSBO.Create(renderer->GetDevice(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	mCounterSSBO.UpdateMemory(renderer->GetVkDevice());

	uint32_t size = blockSize*blockSize*blockSize * 5 * 3;
	mVertexBuffer = new Vulkan::Buffer(renderer->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size, nullptr);
	mBufferInfo.buffer = mVertexBuffer->GetVkBuffer();
	mBufferInfo.range = size;
	mBufferInfo.offset = 0;

	// TODO: Bad
	mDescriptorSet = new Vulkan::DescriptorSet(renderer->GetDevice(), desscriptorSetLayout, descriptorPool);
	mDescriptorSet->AllocateDescriptorSets();
	mDescriptorSet->BindStorageBuffer(0, &mBufferInfo);
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

bool Block::IsVisible()
{
	return mVisible;
}

void Block::SetGenerated(bool generated)
{
	mGenerated = generated;
}

void Block::SetModified(bool modified)
{
	mModified = modified;
}

void Block::SetVisible(bool visible)
{
	mVisible = visible;
}

void Block::SetNumVertices(uint32_t numVertices)
{
	mNumVertices = numVertices;
}

void Block::SetColor(glm::vec3 color)
{
	mColor = color;
}

glm::vec3 Block::GetPosition()
{
	return mPosition;
}

glm::vec3 Block::GetColor()
{
	return mColor;
}

Vulkan::DescriptorSet* Block::GetDescriptorSet()
{
	return mDescriptorSet;
}

uint32_t Block::GetNumVertices()
{
	return mNumVertices;
}

