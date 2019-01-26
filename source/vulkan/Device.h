#pragma once

#include <vulkan\vulkan.h>
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class Device
	{
	public:
		Device(Instance* instance, bool enableValidation = false);
		~Device();
		
		void Create(Instance* instance, bool enableValidation);

		// [TODO] Add creation of the logical device
		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
		VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex);

		VkPhysicalDevice GetPhysicalDevice();
		VkDevice GetVkDevice();

		uint32_t GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex);
		VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties();

		Queue* GetQueue() const;

	private:
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;
		VkCommandPool mCommandPool = VK_NULL_HANDLE;

		Queue* mQueue;

		// Stores all available memory (type) properties for the physical device
		VkPhysicalDeviceMemoryProperties mDeviceMemoryProperties;

		VkPhysicalDeviceFeatures mEnabledFeatures{};
	};
}
