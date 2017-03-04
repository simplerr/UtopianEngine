#include "VertexUniformBuffer.h"
#include "VulkanDebug.h"
#include "vulkan/handles/Buffer.h"

namespace Vulkan
{
	void VertexUniformBuffer::UpdateMemory(VkDevice device)
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(camera), 0, (void**)&mapped);
		memcpy(mapped, &camera, sizeof(camera));
		mBuffer->UnmapMemory();
	}

	int VertexUniformBuffer::GetSize()
	{
		return sizeof(camera) + sizeof(constants);
	}
}