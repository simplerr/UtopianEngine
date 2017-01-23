#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Fence : public Handle<VkFence>
	{
	public:
		Fence(VkDevice device, VkFenceCreateFlags flags);

		void Create(VkFenceCreateFlags flags);
		void Wait();
		void Reset();
	private:
	};
}
