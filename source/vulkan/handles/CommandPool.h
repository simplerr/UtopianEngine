#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h" 

namespace VulkanLib
{
	class Device;

	class CommandPool : public Handle<VkCommandPool>
	{
	public:
		CommandPool();
		CommandPool(Device* device, uint32_t queueFamilyIndex = 0);

		void Create(Device* device, uint32_t queueFamilyIndex = 0);
	private:
	};
}
