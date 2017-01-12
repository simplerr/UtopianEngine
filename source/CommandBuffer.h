#pragma once
#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class VulkanDevice;
	class CommandPool;

	class CommandBuffer : public Handle<VkCommandBuffer>
	{
	public:
		CommandBuffer(VulkanDevice* device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		void Begin();
		void Begin(VkRenderPass renderPass, VkFramebuffer frameBuffer);
		void End();
		void Flush(VulkanDevice* device, VkQueue queue, CommandPool* commandPool, bool free = false);
		void Cleanup(VulkanDevice* device, CommandPool* commandPool);
	private:
	};
}
