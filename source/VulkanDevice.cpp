#include "VulkanDevice.h"
#include "VulkanDebug.h"
#include <stdexcept>

namespace VulkanLib
{
	VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
	{
		mPhysicalDevice = physicalDevice;
		mLogicalDevice = logicalDevice;

		mCommandPool = CreateCommandPool(0); // [NOTE] Hardcoded
	}

	uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	VkCommandPool VulkanDevice::CreateCommandPool(uint32_t queueFamilyIndex)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VkCommandPool cmdPool;
		VulkanDebug::ErrorCheck(vkCreateCommandPool(mLogicalDevice, &cmdPoolInfo, nullptr, &cmdPool));
		return cmdPool;
	}

	VkCommandBuffer VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		VkCommandBuffer cmdBuffer;
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = level;

		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mLogicalDevice, &allocateInfo, &cmdBuffer));

		// If requested, also start the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VulkanDebug::ErrorCheck(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
		}

		return cmdBuffer;
	}

	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
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
			vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, &commandBuffer);
		}
	}
	VkPhysicalDevice VulkanDevice::GetPhysicalDevice()
	{
		return mPhysicalDevice;
	}

	VkDevice VulkanDevice::GetLogicalDevice()
	{
		return mLogicalDevice;
	}
}
