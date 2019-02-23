#pragma once

#include "utility\Module.h"
#include "utility\Common.h"
#include "vulkan\VulkanInclude.h"

namespace Utopian
{
	struct CopyImageInfo
	{
		// Attributes describing the destination image
		uint32_t width;
		uint32_t height;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags memoryProperties;
		VkImageLayout finalImageLayout;
	};

	class RendererUtility : public Module<RendererUtility>
	{
	public:
		void DrawFullscreenQuad(Vk::CommandBuffer* commandBuffer);
		//void DrawMesh(...);

		SharedPtr<Vk::Image> CopyImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage, const CopyImageInfo& info);
		void PrepareForTransferDst(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& dstImage);
		void PrepareForTransferSrc(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& srcImage);
		void PrepareForRead(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& dstImage);
		void RestoreLayout(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& srcImage);
		void InsertMemoryBarrier(const Vk::CommandBuffer& commandBuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout);
	private:

	};

	RendererUtility& gRendererUtility();
}
