#pragma once
#include <vulkan\vulkan.h>

namespace VulkanLib
{
	class Device
	{
	public:
		Device(VkInstance instance, bool enableValidation = false);
		~Device();
		
		void Create(VkInstance instance, bool enableValidation);

		// [TODO] Add creation of the logical device
		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free);
		VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		VkPhysicalDevice GetPhysicalDevice();
		VkDevice GetVkDevice();
	private:
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;
		VkCommandPool mCommandPool = VK_NULL_HANDLE;
	};
}
