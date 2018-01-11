#pragma once

#include "Handle.h" 
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	class CommandPool : public Handle<VkCommandPool>
	{
	public:
		CommandPool();
		CommandPool(Device* device, uint32_t queueFamilyIndex = 0);

		void Create(Device* device, uint32_t queueFamilyIndex = 0);
	private:
	};
}
