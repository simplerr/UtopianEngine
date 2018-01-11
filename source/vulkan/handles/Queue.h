#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	class Queue : public Handle<VkQueue>
	{
	public:
		Queue(Device* device, Semaphore* waitSemaphore, Semaphore* signalSemaphore);
		void Create(VkSemaphore* waitSemaphore, VkSemaphore* signalSemaphore);
		void Submit(CommandBuffer* commandBuffer, Fence* renderFence);
		void WaitIdle();
	protected:
		VkSubmitInfo mSubmitInfo = {};
		VkPipelineStageFlags mStageFlags = {};
	};
}
