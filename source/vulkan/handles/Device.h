#pragma once

#include <vulkan/vulkan.h>
#include "vulkan/VulkanInclude.h"
#include <vector>
#include <string>

namespace Utopian::Vk
{
	struct VulkanVersion
	{
		VulkanVersion();
		VulkanVersion(uint32_t apiVersion);

		uint32_t major;
		uint32_t minor;
		uint32_t patch;
		std::string version;
	};

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

		/** Returns the command pool from the device which new command buffers can be allocated from. */
		CommandPool* GetCommandPool() const;

		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetVkDevice() const;
		VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties() const;
		uint32_t GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex) const;
		bool IsDebugMarkersEnabled() const;
		uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const;
		VulkanVersion GetVulkanVersion() const;

	private:
		void RetrievePhysical(Instance* instance);
		void RetrieveQueueFamilyProperites();
		void CreateLogical(bool enableValidation);
		void RetrieveSupportedExtensions();
		bool IsExtensionSupported(std::string extension);

	private:
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties mPhysicalDeviceProperties;
		VkPhysicalDeviceMemoryProperties mDeviceMemoryProperties;
		VkPhysicalDeviceFeatures mEnabledFeatures {};
		VkPhysicalDeviceFeatures mAvailableFeatures;
		std::vector<std::string> mSupportedExtensions;
		std::vector<VkQueueFamilyProperties> mQueueFamilyProperties;
		VulkanVersion mVulkanVersion;

		CommandPool* mCommandPool = nullptr;
		Queue* mQueue = nullptr;
		bool mDebugMarkersEnabled = false;
	};
}
