#pragma once
#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Fence : public Handle<VkFence>
	{
	public:
		Fence();

		void Create(VkDevice device, VkFenceCreateFlags flags);
		void Wait(VkDevice device);
		void Reset(VkDevice device);
	private:
	};
}
