#include "FragmentUniformBuffer.h"
#include "VulkanDebug.h"

void FragmentUniformBuffer::UpdateMemory(VkDevice device)
{
	// Map and update the light data
	uint8_t* data;
	uint32_t dataOffset = 0;
	uint32_t dataSize = lights.size() * sizeof(VulkanLib::Light);
	VulkanLib::VulkanDebug::ErrorCheck(vkMapMemory(device, mMemory, dataOffset, dataSize, 0, (void **)&data));
	memcpy(data, lights.data(), dataSize);
	vkUnmapMemory(device, mMemory);

	// Map and update number of lights
	uint8_t* data1;
	dataOffset += dataSize;
	dataSize = sizeof(constants);
	VulkanLib::VulkanDebug::ErrorCheck(vkMapMemory(device, mMemory, dataOffset, dataSize, 0, (void **)&data1));
	memcpy(data1, &constants.numLights, dataSize);
	vkUnmapMemory(device, mMemory);
}

int FragmentUniformBuffer::GetSize()
{
	return lights.size() * sizeof(VulkanLib::Light) + sizeof(constants);
}