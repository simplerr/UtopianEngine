#include "Fence.h"
#include "VulkanDebug.h"

namespace VulkanLib
{
	Fence::Fence()
		: Handle(vkDestroyFence)
	{

	}

	void Fence::Create(VkDevice device, VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = flags;

		vkCreateFence(device, &fenceCreateInfo, NULL, &mHandle);
	}

	void Fence::Wait(VkDevice device)
	{
		// Wait for fence to signal that all command buffers are ready
		VkResult fenceRes;
		do
		{
			fenceRes = vkWaitForFences(device, 1, &mHandle, VK_TRUE, 100000000);
		} while (fenceRes == VK_TIMEOUT);

		VulkanDebug::ErrorCheck(fenceRes);
		Reset(device);
	}

	void Fence::Reset(VkDevice device)
	{
		vkResetFences(device, 1, &mHandle);
	}
}
