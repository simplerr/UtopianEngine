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
		CommandPool(VkDevice device, uint32_t queueFamilyIndex = 0);

		void Create(VkDevice device, uint32_t queueFamilyIndex = 0);
	private:
	};
}
