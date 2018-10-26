#include "Semaphore.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"

namespace Utopian::Vk
{
	Semaphore::Semaphore(Device* device)
		: Handle(device, vkDestroySemaphore)
	{
		VkSemaphoreCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VulkanDebug::ErrorCheck(vkCreateSemaphore(GetVkDevice(), &createInfo, nullptr, &mHandle));
	}
}