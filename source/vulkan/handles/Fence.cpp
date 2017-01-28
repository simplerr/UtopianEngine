#include "vulkan/VulkanDebug.h"
#include "Fence.h"

namespace VulkanLib
{
	Fence::Fence(VkDevice device, VkFenceCreateFlags flags)
		: Handle(device, vkDestroyFence)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = flags;

		vkCreateFence(GetDevice(), &fenceCreateInfo, NULL, &mHandle);
	}

	void Fence::Create(VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = flags;

		vkCreateFence(GetDevice(), &fenceCreateInfo, NULL, &mHandle);
	}

	void Fence::Wait()
	{
		// Wait for fence to signal that all command buffers are ready
		VkResult fenceRes;
		do
		{
			fenceRes = vkWaitForFences(GetDevice(), 1, &mHandle, VK_TRUE, 100000000);
		} while (fenceRes == VK_TIMEOUT);

		VulkanDebug::ErrorCheck(fenceRes);

		Reset();
	}

	void Fence::Reset()
	{
		vkResetFences(GetDevice(), 1, &mHandle);
	}
}