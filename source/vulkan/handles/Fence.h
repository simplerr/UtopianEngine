#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Device;

	class Fence : public Handle<VkFence>
	{
	public:
		Fence(Device* device, VkFenceCreateFlags flags);

		void Create(VkFenceCreateFlags flags);
		void Wait();
		void Reset();
	private:
	};
}
