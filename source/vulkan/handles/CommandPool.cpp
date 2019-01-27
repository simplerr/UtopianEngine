#include "vulkan/VulkanDebug.h"
#include "vulkan/Device.h"
#include "CommandPool.h"

namespace Utopian::Vk
{
	CommandPool::CommandPool()
		: Handle(vkDestroyCommandPool)
	{

	}

	CommandPool::CommandPool(Device* device, uint32_t queueFamilyIndex)
		: Handle(device, vkDestroyCommandPool)
	{
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamilyIndex;									// NOTE: TODO: Need to store this as a member (Use Swapchain)!!!!!
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VulkanDebug::ErrorCheck(vkCreateCommandPool(device->GetVkDevice(), &createInfo, nullptr, &mHandle));
	}
}
