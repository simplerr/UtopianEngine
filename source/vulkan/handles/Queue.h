#pragma once

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
		void Submit(CommandBuffer* commandBuffer, VkSubmitInfo submitInfo);
		void Submit(CommandBuffer* commandBuffer, Semaphore* waitSemaphore, Semaphore* signalSemaphore, VkPipelineStageFlags stageFlags);
		void WaitIdle();

		void SetWaitSemaphore(Semaphore* semaphore);
		void SetSignalSemaphore(Semaphore* semaphore);
	protected:
		VkSubmitInfo mSubmitInfo = {};
		VkPipelineStageFlags mStageFlags = {};
	};
}
