#pragma once

#include "utility\Module.h"
#include "utility\Common.h"
#include "vulkan\VulkanInclude.h"
#include <string>

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

		/** Blend state helpers. */
		void SetAdditiveBlending(Vk::Pipeline* pipeline);
		void SetAlphaBlending(Vk::Pipeline* pipeline);

		/** Functions for copying images. */
		void SaveToFile(Vk::Device* device, const SharedPtr<Vk::Image>& image, std::string filename, uint32_t width, uint32_t height);
		SharedPtr<Vk::Image> CopyImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage, const CopyImageInfo& info);
		SharedPtr<Vk::Image> CreateHostVisibleImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage, uint32_t width, uint32_t height, VkFormat format);

	private:
		void PrepareForTransferDst(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& dstImage);
		void PrepareForTransferSrc(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& srcImage);
		void PrepareForRead(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& dstImage);
		void RestoreLayout(const Vk::CommandBuffer& commandBuffer, const SharedPtr<Vk::Image>& srcImage);
		void InsertMemoryBarrier(const Vk::CommandBuffer& commandBuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout);
	};

	RendererUtility& gRendererUtility();
}
