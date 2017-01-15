#pragma once
#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Device;
	class CommandPool;

	class CommandBuffer : public Handle<VkCommandBuffer>
	{
	public:
		CommandBuffer();
		CommandBuffer(VkDevice device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		void Create(VkDevice device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		void Begin();
		void Begin(VkRenderPass renderPass, VkFramebuffer frameBuffer);
		void End();
		void Flush(VkDevice device, VkQueue queue, CommandPool* commandPool, bool free = false);
		void Cleanup(VkDevice device, CommandPool* commandPool);
	private:
	};
}
