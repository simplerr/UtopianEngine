#pragma once

#include "Handle.h" 
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	/** Wrapper for VkCommandPool. */
	class CommandPool : public Handle<VkCommandPool>
	{
	public:
		CommandPool(Device* device, uint32_t queueFamilyIndex = 0);
	private:
	};
}
