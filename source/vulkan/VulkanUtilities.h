#pragma once
#include <vulkan/vulkan.h>
#include "vulkan/Device.h"
#include "vulkan/handles/Queue.h"

namespace Utopian::Vk
{
	namespace Utilities
	{
		void CopyBufferToImage(Device* device, Queue* queue, Buffer* buffer, Image* image, uint32_t regionCount, const VkBufferImageCopy* regions, VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			vkCmdCopyBufferToImage(
				commandBuffer,
				buffer->GetVkBuffer(),
				image->GetVkHandle(),
				dstImageLayout,
				regionCount,
				regions);

			device->FlushCommandBuffer(commandBuffer, queue->GetVkHandle(), true);
		}

		void TransitionImageLayout(Device* device, Queue* queue, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
		{
			VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange = subresourceRange;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
				barrier.srcAccessMask = 0; // Note needed when old layout is undefined
			else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			else
				assert(0);

			if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			else if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			else
				assert(0);

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			device->FlushCommandBuffer(commandBuffer, queue->GetVkHandle(), true);
		}
	}
}
