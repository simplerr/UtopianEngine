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
		Queue();
		void Create(VkDevice device, VkSemaphore* waitSemaphore, VkSemaphore* signalSemaphore);
		void Submit(CommandBuffer* commandBuffer, Fence* renderFence);
		void WaitIdle();
	protected:
		VkSubmitInfo mSubmitInfo = {};
		VkPipelineStageFlags mStageFlags = {};
	};
}
