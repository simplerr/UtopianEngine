#pragma once

#include "Handle.h" 
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
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
