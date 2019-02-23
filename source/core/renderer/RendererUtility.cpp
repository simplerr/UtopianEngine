#include "core/renderer/RendererUtility.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"

namespace Utopian
{
	RendererUtility& gRendererUtility()
	{
		return RendererUtility::Instance();
	}

	void RendererUtility::DrawFullscreenQuad(Vk::CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdDraw(3, 1, 0, 0);
	}

	SharedPtr<Vk::Image> RendererUtility::CopyImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage, const CopyImageInfo& info)
	{
		SharedPtr<Vk::Image> dstImage = std::make_shared<Vk::Image>(device, info.width, info.height, info.format, info.tiling, info.usage, info.memoryProperties);
		dstImage->SetFinalLayout(info.finalImageLayout);

		VkFormatProperties formatProps;

		// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
		vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), srcImage->GetFormat(), &formatProps);
		if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
			assert(0);
		}

		// Check if the device supports blitting to linear images 
		vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), dstImage->GetFormat(), &formatProps);
		if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
			assert(0);
		}

		Vk::CommandBuffer commandBuffer = Vk::CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		PrepareForTransferDst(commandBuffer, dstImage);
		PrepareForTransferSrc(commandBuffer, srcImage);

		// Blit
		VkOffset3D srcBlitSize;
		srcBlitSize.x = srcImage->GetWidth();
		srcBlitSize.y = srcImage->GetHeight();
		srcBlitSize.z = 1;
		VkOffset3D dstBlitSize;
		dstBlitSize.x = dstImage->GetWidth();
		dstBlitSize.y = dstImage->GetHeight();
		dstBlitSize.z = 1;
		VkImageBlit imageBlitRegion{};
		imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlitRegion.srcSubresource.layerCount = 1;
		imageBlitRegion.srcOffsets[1] = srcBlitSize;
		imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlitRegion.dstSubresource.layerCount = 1;
		imageBlitRegion.dstOffsets[1] = dstBlitSize;

		vkCmdBlitImage(
			commandBuffer.GetVkHandle(),
			srcImage->GetVkHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage->GetVkHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageBlitRegion,
			VK_FILTER_NEAREST);

		PrepareForRead(commandBuffer, dstImage);
		RestoreLayout(commandBuffer, srcImage);

		commandBuffer.Flush();

		return dstImage;
	}

	void RendererUtility::PrepareForTransferDst(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& dstImage)
	{
		InsertMemoryBarrier(commandBuffer, dstImage->GetVkHandle(), 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	void RendererUtility::PrepareForTransferSrc(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& srcImage)
	{
		InsertMemoryBarrier(commandBuffer, srcImage->GetVkHandle(), VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, srcImage->GetFinalLayout(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}

	void RendererUtility::PrepareForRead(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& dstImage)
	{
		InsertMemoryBarrier(commandBuffer, dstImage->GetVkHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImage->GetFinalLayout());
	}

	void RendererUtility::RestoreLayout(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& srcImage)
	{
		InsertMemoryBarrier(commandBuffer, srcImage->GetVkHandle(), VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImage->GetFinalLayout());
	}
	
	void RendererUtility::InsertMemoryBarrier(const Vk::CommandBuffer& commandBuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		vkCmdPipelineBarrier(
			commandBuffer.GetVkHandle(),
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}
}