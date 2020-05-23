#include "Block.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/Debug.h"
#include "vulkan/MarchingCubesTerrainEffect.h"
#include "vulkan/handles/DescriptorSet.h"

Block::Block(Utopian::Vk::Device* device, glm::vec3 position, glm::vec3 color, uint32_t blockSize, float voxelSize, const Utopian::Vk::DescriptorSetLayout* desscriptorSetLayout, Utopian::Vk::DescriptorPool* descriptorPool)
{
	mPosition = position;
	mColor = color;
	mGenerated = false;
	mModified = false;
	mVoxelSize = voxelSize;

	uint32_t size = blockSize*blockSize*blockSize * 5 * 3;
	mVertexBuffer = new Utopian::Vk::Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size, nullptr);
	mBufferInfo.buffer = mVertexBuffer->GetVkBuffer();
	mBufferInfo.range = size;
	mBufferInfo.offset = 0;

	pipelineType = Utopian::Vk::MarchingCubesTerrainEffect::PipelineType2::SOLID;
	//if (rand() % 2 == 0)
	//	pipelineType = Vulkan::TerrainEffect::PipelineType2::WIREFRAME;
	//else
	//pipelineType = Vulkan::TerrainEffect::PipelineType2::SOLID;
}

Block::~Block()
{
	delete mVertexBuffer;
}

VkDescriptorBufferInfo Block::GetBufferInfo()
{
	return mBufferInfo;
}

Utopian::Vk::Buffer* Block::GetVertexBuffer()
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

uint32_t Block::GetNumVertices()
{
	return mNumVertices;
}
