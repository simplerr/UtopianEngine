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
		CommandBuffer(VkDevice device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		~CommandBuffer();

		void Create(CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		void Begin();
		void Begin(VkRenderPass renderPass, VkFramebuffer frameBuffer);
		void End();
		void Flush(VkQueue queue, CommandPool* commandPool, bool free = false);
		void Cleanup(CommandPool* commandPool);
	private:
		CommandPool* mCommandPool; // The command pool this command buffer comes from 
	};
}
