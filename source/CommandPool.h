#pragma once
#include <vulkan/vulkan.h>
#include "Handle.h" 

namespace VulkanLib
{
	class VulkanDevice;

	class CommandPool : public Handle<VkCommandPool>
	{
	public:
		CommandPool(VulkanDevice* device, uint32_t queueFamilyIndex = 0);

		void Cleanup(VulkanDevice* device);
	private:
	};
}
