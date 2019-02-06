#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/handles/Semaphore.h"
#include "utility/Common.h"

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
		void Submit(CommandBuffer* commandBuffer, Fence* renderFence, const SharedPtr<Semaphore>& waitSemaphore, const SharedPtr<Semaphore>& signalSemaphore);
		void WaitIdle();

		const SharedPtr<Semaphore>& GetWaitSemaphore() const;
		const SharedPtr<Semaphore>& GetSignalSemaphore() const;

	protected:
		SharedPtr<Semaphore> mPresentComplete = nullptr;
		SharedPtr<Semaphore> mRenderComplete = nullptr;
		VkSubmitInfo mSubmitInfo = {};
		VkPipelineStageFlags mStageFlags = {};
	};
}
