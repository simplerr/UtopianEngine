#include "Semaphore.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"

namespace VulkanLib
{
	Semaphore::Semaphore(Device* device)
		: Handle(device->GetVkDevice(), vkDestroySemaphore)
	{
		VkSemaphoreCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VulkanDebug::ErrorCheck(vkCreateSemaphore(GetDevice(), &createInfo, nullptr, &mHandle));
	}
}