#include "CommandBuffer.h"
#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "CommandPool.h"

namespace VulkanLib
{
	CommandBuffer::CommandBuffer(VulkanDevice* device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin)
	{
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = commandPool->GetVkHandle();
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = level;

		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(device->GetLogicalDevice(), &allocateInfo, &mHandle));

		// If requested, also start the new command buffer
		if (begin)
		{
			Begin();
		}
	}

	void CommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mHandle, &beginInfo));
	}

	// Used when beginning secondary command buffers
	void CommandBuffer::Begin(VkRenderPass renderPass, VkFramebuffer frameBuffer)
	{
		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = renderPass;
		inheritanceInfo.framebuffer = frameBuffer;

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mHandle, &beginInfo));
	}

	void CommandBuffer::End()
	{
		vkEndCommandBuffer(mHandle);
	}

	void CommandBuffer::Flush(VulkanDevice* device, VkQueue queue, CommandPool* commandPool, bool free)
	{
		if (mHandle == VK_NULL_HANDLE)
		{
			return;
		}

		VulkanDebug::ErrorCheck(vkEndCommandBuffer(mHandle));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mHandle;

		VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(queue));	// [TODO] Wait for fence instead?

		if (free)
		{
			Cleanup(device, commandPool);
		}
	}
	void CommandBuffer::Cleanup(VulkanDevice* device, CommandPool* commandPool)
	{
		vkFreeCommandBuffers(device->GetLogicalDevice(), commandPool->GetVkHandle(), 1, &mHandle);
	}
}
