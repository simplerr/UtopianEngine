#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class Queue : public Handle<VkQueue>
	{
	public:
		Queue(Device* device);
		~Queue();

		/**
		 * @param useSemaphores Controls if the wait and signal semaphores associated
		 * with the Queue should be used or not.
		 */
		void Submit(CommandBuffer* commandBuffer, Fence* renderFence, bool useSemaphores);
		void WaitIdle();

		Semaphore* GetWaitSemaphore() const;
		Semaphore* GetSignalSemaphore() const;

	protected:
		Semaphore* mPresentComplete = nullptr;
		Semaphore* mRenderComplete = nullptr;
		VkSubmitInfo mSubmitInfo = {};
		VkPipelineStageFlags mStageFlags = {};
	};
}
