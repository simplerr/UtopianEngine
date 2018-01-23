#include "vulkan/VulkanDebug.h"
#include "vulkan/Device.h"
#include "Queue.h"
#include "Fence.h"
#include "CommandBuffer.h"
#include "Semaphore.h"

namespace Vulkan
{
	Queue::Queue(Device* device, Semaphore* waitSemaphore, Semaphore* signalSemaphore)
		: Handle(device, nullptr)
	{
		mSubmitInfo = {};
		mStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		// Setup default submit info
		mSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		mSubmitInfo.waitSemaphoreCount = 1;
		mSubmitInfo.signalSemaphoreCount = 1;
		mSubmitInfo.pWaitSemaphores = &waitSemaphore->mHandle;							// Waits for swapChain.acquireNextImage to complete
		mSubmitInfo.pSignalSemaphores = &signalSemaphore->mHandle;						// swapChain.queuePresent will wait for this submit to complete
		mSubmitInfo.pWaitDstStageMask = &mStageFlags;

		// Get the queue from the device
		// [NOTE] that queueFamilyIndex is hard coded to 0
		vkGetDeviceQueue(GetDevice(), 0, 0, &mHandle);
	}

	void Queue::Create(VkSemaphore* waitSemaphore, VkSemaphore* signalSemaphore)
	{
		// Setup default submit info
		mSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		mSubmitInfo.waitSemaphoreCount = 1;
		mSubmitInfo.signalSemaphoreCount = 1;
		mSubmitInfo.pWaitSemaphores = waitSemaphore;							// Waits for swapChain.acquireNextImage to complete
		mSubmitInfo.pSignalSemaphores = signalSemaphore;						// swapChain.queuePresent will wait for this submit to complete
		mSubmitInfo.pWaitDstStageMask = &mStageFlags;

		// Get the queue from the device
		// [NOTE] that queueFamilyIndex is hard coded to 0
		vkGetDeviceQueue(GetDevice(), 0, 0, &mHandle);
	}

	void Queue::Submit(CommandBuffer* commandBuffer, Fence* renderFence)
	{
		mSubmitInfo.pCommandBuffers = &commandBuffer->mHandle;					// Draw commands for the current command buffer
		mSubmitInfo.commandBufferCount = 1;

		if (renderFence == nullptr)
			VulkanDebug::ErrorCheck(vkQueueSubmit(GetVkHandle(), 1, &mSubmitInfo, VK_NULL_HANDLE));
		else
			VulkanDebug::ErrorCheck(vkQueueSubmit(GetVkHandle(), 1, &mSubmitInfo, renderFence->GetVkHandle()));
	}

	void Queue::Submit(CommandBuffer* commandBuffer, VkSubmitInfo submitInfo)
	{
		mSubmitInfo = submitInfo;
		mSubmitInfo.pCommandBuffers = &commandBuffer->mHandle;					// Draw commands for the current command buffer
		mSubmitInfo.commandBufferCount = 1;

		VulkanDebug::ErrorCheck(vkQueueSubmit(GetVkHandle(), 1, &mSubmitInfo, VK_NULL_HANDLE));
	}

	void Queue::Submit(CommandBuffer* commandBuffer, Semaphore* waitSemaphore, Semaphore* signalSemaphore, VkPipelineStageFlags stageFlags)
	{
		// Setup default submit info
		mSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		mSubmitInfo.waitSemaphoreCount = 1;
		mSubmitInfo.signalSemaphoreCount = 1;
		mSubmitInfo.pWaitSemaphores = &waitSemaphore->mHandle;							// Waits for swapChain.acquireNextImage to complete
		mSubmitInfo.pSignalSemaphores = &signalSemaphore->mHandle;						// swapChain.queuePresent will wait for this submit to complete
		mSubmitInfo.pWaitDstStageMask = &stageFlags;
		VulkanDebug::ErrorCheck(vkQueueSubmit(GetVkHandle(), 1, &mSubmitInfo, VK_NULL_HANDLE));
	}

	void Queue::WaitIdle()
	{
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(GetVkHandle()));
	}

	void Queue::SetWaitSemaphore(Semaphore* semaphore)
	{
		mSubmitInfo.pWaitSemaphores = &semaphore->mHandle;
		mSubmitInfo.waitSemaphoreCount = 1;
	}

	void Queue::SetSignalSemaphore(Semaphore* semaphore)
	{
		mSubmitInfo.pSignalSemaphores = &semaphore->mHandle;
		mSubmitInfo.signalSemaphoreCount = 1;
	}
}