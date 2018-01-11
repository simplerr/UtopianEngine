#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
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
