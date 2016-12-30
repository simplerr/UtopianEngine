#include "BigUniformBuffer.h"
#include "VulkanDebug.h"

void BigUniformBuffer::UpdateMemory(VkDevice device)
{
	// Map uniform buffer and update it
	uint8_t *data1;
	VulkanLib::VulkanDebug::ErrorCheck(vkMapMemory(device, mMemory, 0, sizeof(camera), 0, (void **)&data1));
	memcpy(data1, &camera, sizeof(camera));
	vkUnmapMemory(device, mMemory);

	// Map and update the light data
	uint8_t *data2;
	uint32_t dataOffset = sizeof(camera);
	uint32_t dataSize = lights.size() * sizeof(VulkanLib::Light);
	VulkanLib::VulkanDebug::ErrorCheck(vkMapMemory(device, mMemory, dataOffset, dataSize, 0, (void **)&data2));
	memcpy(data2, lights.data(), dataSize);
	vkUnmapMemory(device, mMemory);

	// Map and update number of lights
	dataOffset += dataSize;
	dataSize = sizeof(constants);
	uint8_t *data3;
	VulkanLib::VulkanDebug::ErrorCheck(vkMapMemory(device, mMemory, dataOffset, dataSize, 0, (void **)&data3));
	memcpy(data3, &constants.numLights, dataSize);
	vkUnmapMemory(device, mMemory);
}

int BigUniformBuffer::GetSize()
{
	return sizeof(camera) + lights.size() * sizeof(VulkanLib::Light) + sizeof(constants);
}