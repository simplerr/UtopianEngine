#include "vulkan/VulkanDebug.h"
#include "vulkan/Device.h"
#include "Fence.h"

namespace Utopian::Vk
{
	Fence::Fence(Device* device, VkFenceCreateFlags flags)
		: Handle(device, vkDestroyFence)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = flags;

		vkCreateFence(GetVkDevice(), &fenceCreateInfo, NULL, &mHandle);
	}

	void Fence::Create(VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = flags;

		vkCreateFence(GetVkDevice(), &fenceCreateInfo, NULL, &mHandle);
	}

	void Fence::Wait()
	{
		// Wait for fence to signal that all command buffers are ready
		VkResult fenceRes;
		do
		{
			fenceRes = vkWaitForFences(GetVkDevice(), 1, &mHandle, VK_TRUE, 100000000);
		} while (fenceRes == VK_TIMEOUT);

		VulkanDebug::ErrorCheck(fenceRes);

		Reset();
	}

	void Fence::Reset()
	{
		vkResetFences(GetVkDevice(), 1, &mHandle);
	}
}
