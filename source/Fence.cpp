#include "Fence.h"

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

	void Fence::Reset(VkDevice device)
	{
		vkResetFences(device, 1, &mHandle);
	}
}
