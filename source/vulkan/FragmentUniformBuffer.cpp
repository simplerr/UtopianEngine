#include "FragmentUniformBuffer.h"
#include "VulkanDebug.h"
#include "vulkan/handles/Buffer.h"

void FragmentUniformBuffer::UpdateMemory(VkDevice device)
{
	// Map and update the light data
	uint8_t* mapped;
	uint32_t dataOffset = 0;
	uint32_t dataSize = lights.size() * sizeof(Vulkan::LightData);
	mBuffer->MapMemory(dataOffset, dataSize, 0, (void**)&mapped);
	memcpy(mapped, lights.data(), dataSize);
	mBuffer->UnmapMemory();

	// Map and update number of lights
	dataOffset += dataSize;
	dataSize = sizeof(constants);
	mBuffer->MapMemory(dataOffset, dataSize, 0, (void**)&mapped);
	memcpy(mapped, &constants.numLights, dataSize);
	mBuffer->UnmapMemory();
}

int FragmentUniformBuffer::GetSize()
{
	return lights.size() * sizeof(Vulkan::LightData) + sizeof(constants);
}