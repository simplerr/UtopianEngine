#include <stdexcept>
#include <array>
#include "Device.h"
#include "handles/Instance.h"
#include "handles/Queue.h"
#include "handles/CommandPool.h"
#include "VulkanDebug.h"

namespace Utopian::Vk
{
	Device::Device(Instance* instance, bool enableValidation)
	{
		mEnabledFeatures.geometryShader = VK_TRUE;
		mEnabledFeatures.shaderTessellationAndGeometryPointSize = VK_TRUE;
		mEnabledFeatures.fillModeNonSolid = VK_TRUE;

		Create(instance, enableValidation);

		mCommandPool = new CommandPool(this, 0);
		mQueue = new Queue(this);

		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mDeviceMemoryProperties);
	}

	Device::~Device()
	{
		vkDestroyCommandPool(mDevice, mCommandPool->GetVkHandle(), nullptr);
		vkDestroyDevice(mDevice, nullptr);

		delete mQueue;
	}

	void Device::Create(Instance* instance, bool enableValidation)
	{
		// Query for the number of GPUs
		uint32_t gpuCount = 0;
		VkResult result = vkEnumeratePhysicalDevices(instance->GetVkHandle(), &gpuCount, NULL);

		if (result != VK_SUCCESS)
			VulkanDebug::ConsolePrint("vkEnumeratePhysicalDevices failed");

		if (gpuCount < 1)
			VulkanDebug::ConsolePrint("vkEnumeratePhysicalDevices didn't find any valid devices for Vulkan");

		// Enumerate devices
		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		result = vkEnumeratePhysicalDevices(instance->GetVkHandle(), &gpuCount, physicalDevices.data());

		// Assume that there only is 1 GPU
		mPhysicalDevice = physicalDevices[0];

		// This is not used right now, but GPU vendor and model can be retrieved
		//VkPhysicalDeviceProperties physicalDeviceProperties;
		//vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		// Some implementations use vkGetPhysicalDeviceQueueFamilyProperties and uses the result to find out
		// the first queue that support graphic operations (queueFlags & VK_QUEUE_GRAPHICS_BIT)
		// Here I simply set queueInfo.queueFamilyIndex = 0 and (hope) it works
		// In Sascha Willems examples he has a compute queue with queueFamilyIndex = 1

		std::array<float, 1> queuePriorities = { 1.0f };
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pNext = nullptr;
		queueInfo.flags = 0;
		queueInfo.queueFamilyIndex = 0; // 0 seems to always be the first valid queue (see above)
		queueInfo.pQueuePriorities = queuePriorities.data();
		queueInfo.queueCount = 1;

		// Use the VK_KHR_SWAPCHAIN_EXTENSION_NAME extension
		// [NOTE] There is another VK_EXT_DEBUG_MARKER_EXTENSION_NAME extension that might be good to add later.
		std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = nullptr;
		deviceInfo.flags = 0;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.pEnabledFeatures = &mEnabledFeatures;
		deviceInfo.enabledExtensionCount = enabledExtensions.size();				// Extensions
		deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();

		if (enableValidation)
		{
			deviceInfo.enabledLayerCount = VulkanDebug::validation_layers.size();	// Debug validation layers
			deviceInfo.ppEnabledLayerNames = VulkanDebug::validation_layers.data();
		}

		VulkanDebug::ErrorCheck(vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice));
	}

	VkCommandBuffer Device::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		VkCommandBuffer cmdBuffer;
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPool->GetVkHandle();
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = level;

		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, &cmdBuffer));

		// If requested, also start the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VulkanDebug::ErrorCheck(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
		}

		return cmdBuffer;
	}

	void Device::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		VulkanDebug::ErrorCheck(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(queue));	// [TODO] Wait for fence instead?

		if (free)
		{
			vkFreeCommandBuffers(mDevice, mCommandPool->GetVkHandle(), 1, &commandBuffer);
		}
	}
	VkPhysicalDevice Device::GetPhysicalDevice()
	{
		return mPhysicalDevice;
	}

	VkDevice Device::GetVkDevice()
	{
		return mDevice;
	}

	VkBool32 Device::GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t* typeIndex)
	{
		for (uint32_t i = 0; i < 32; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((mDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					*typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}

		return false;
	}

	VkPhysicalDeviceMemoryProperties Device::GetPhysicalDeviceMemoryProperties()
	{
		return mDeviceMemoryProperties;
	}

	Queue* Device::GetQueue() const
	{
		return mQueue;
	}

	CommandPool* Device::GetCommandPool() const
	{
		return mCommandPool;
	}
}
