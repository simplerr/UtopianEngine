#include "VertexUniformBuffer.h"
#include "VulkanDebug.h"

namespace Vulkan
{
	void VertexUniformBuffer::UpdateMemory(VkDevice device)
	{
		// Map uniform buffer and update it
		uint8_t *data1;
		VulkanDebug::ErrorCheck(vkMapMemory(device, mMemory, 0, sizeof(camera), 0, (void **)&data1));
		memcpy(data1, &camera, sizeof(camera));
		vkUnmapMemory(device, mMemory);
	}

	int VertexUniformBuffer::GetSize()
	{
		return sizeof(camera) + sizeof(constants);
	}
}