#pragma once

#include <vulkan\vulkan.h>
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	/** Wrapper for the Vulkan device. */
	class Device
	{
	public:
		Device(Instance* instance, bool enableValidation = false);
		~Device();

		/**
		 * Returns the graphics queue associated with the device.
		 * 
		 * @note Currently only one queue is fetched from the device.
		 */
		Queue* GetQueue() const;

		/**
		 * Returns the command pool from the device which new
		 * command buffers can be allocated from.
		 */
		CommandPool* GetCommandPool() const;

		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetVkDevice() const;
		VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties() const;
		uint32_t GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex) const;

	private:
		void Create(Instance* instance, bool enableValidation);

	private:
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceMemoryProperties mDeviceMemoryProperties;
		VkPhysicalDeviceFeatures mEnabledFeatures{};

		CommandPool* mCommandPool = nullptr;
		Queue* mQueue = nullptr;
	};
}
