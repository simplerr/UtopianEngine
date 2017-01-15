#pragma once
#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class CommandBuffer;
	class Fence;

	class Queue : public Handle<VkQueue>
	{
	public:
		Queue(VkDevice device, VkSemaphore* waitSemaphore, VkSemaphore* signalSemaphore);
		void Create(VkSemaphore* waitSemaphore, VkSemaphore* signalSemaphore);
		void Submit(CommandBuffer* commandBuffer, Fence* renderFence);
		void WaitIdle();
	protected:
		VkSubmitInfo mSubmitInfo = {};
		VkPipelineStageFlags mStageFlags = {};
	};
}
