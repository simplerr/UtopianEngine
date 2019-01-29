#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	/** Wrapper for VkQueue. */
	class Queue : public Handle<VkQueue>
	{
	public:
		Queue(Device* device);
		~Queue();

		/**
		 * Submits a recorded command buffer to the graphics queue.
		 * 
		 * @param useSemaphores Controls if the wait and signal semaphores associated
		 * with the Queue should be used or not. For example when submitting a command buffer
		 * in offscreen render passes semaphores are not currently used.
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
