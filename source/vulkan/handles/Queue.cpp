#include "vulkan/VulkanDebug.h"
#include "vulkan/handles/Device.h"
#include "Queue.h"
#include "Fence.h"
#include "CommandBuffer.h"
#include "Semaphore.h"

namespace Utopian::Vk
{
	Queue::Queue(Device* device)
		: Handle(device, nullptr)
	{
		mPresentComplete = new Semaphore(device);
		mRenderComplete = new Semaphore(device);

		mSubmitInfo = {};
		mStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		// Setup default submit info
		mSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		mSubmitInfo.waitSemaphoreCount = 1;
		mSubmitInfo.signalSemaphoreCount = 1;
		mSubmitInfo.pWaitSemaphores = &mPresentComplete->mHandle;		// Waits for swapChain.acquireNextImage to complete
		mSubmitInfo.pSignalSemaphores = &mRenderComplete->mHandle;		// swapChain.queuePresent will wait for this submit to complete
		mSubmitInfo.pWaitDstStageMask = &mStageFlags;

		// Get the queue from the device
		// [NOTE] that queueFamilyIndex is hard coded to 0
		vkGetDeviceQueue(GetVkDevice(), 0, 0, &mHandle);
	}

	Queue::~Queue()
	{
		delete mPresentComplete;
		delete mRenderComplete;
	}

	void Queue::Submit(CommandBuffer* commandBuffer, Fence* renderFence, bool useSemaphores)
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		if (useSemaphores)
		{
			submitInfo = mSubmitInfo;
		}

		VkCommandBuffer cmdBuffer = commandBuffer->GetVkHandle();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		if (renderFence == nullptr)
			VulkanDebug::ErrorCheck(vkQueueSubmit(GetVkHandle(), 1, &submitInfo, VK_NULL_HANDLE));
		else
			VulkanDebug::ErrorCheck(vkQueueSubmit(GetVkHandle(), 1, &submitInfo, renderFence->GetVkHandle()));
	}

	void Queue::WaitIdle()
	{
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(GetVkHandle()));
	}

	Semaphore* Queue::GetWaitSemaphore() const
	{
		return mPresentComplete;
	}

	Semaphore* Queue::GetSignalSemaphore() const
	{
		return mRenderComplete;
	}
}