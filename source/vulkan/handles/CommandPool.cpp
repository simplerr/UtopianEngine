#include "vulkan/VulkanDebug.h"
#include "vulkan/handles/Device.h"
#include "CommandPool.h"

namespace Utopian::Vk
{
	CommandPool::CommandPool(Device* device, uint32_t queueFamilyIndex)
		: Handle(device, vkDestroyCommandPool)
	{
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamilyIndex;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VulkanDebug::ErrorCheck(vkCreateCommandPool(device->GetVkDevice(), &createInfo, nullptr, &mHandle));
	}
}
