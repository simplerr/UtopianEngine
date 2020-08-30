#pragma once

#include "utility\Module.h"
#include "utility\Common.h"
#include "vulkan\VulkanPrerequisites.h"
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
		void SetAdditiveBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState);
		void SetAlphaBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState);

		/** Functions for copying images. */
		void CopyImage(Vk::Device* device, Vk::Image& dstImage, Vk::Image& srcImage);

		void SaveToFile(Vk::Device* device, const SharedPtr<Vk::Image>& image, std::string filename, uint32_t width, uint32_t height);

		SharedPtr<Vk::Image> CreateHostVisibleImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage, uint32_t width, uint32_t height, VkFormat format);

	private:
		void SaveToFileKtx(std::string filename, const char* data, uint32_t width, uint32_t height, VkSubresourceLayout layout);
		void SaveToFilePpm(std::string filename, const char* data, uint32_t width, uint32_t height, VkSubresourceLayout layout);
	};

	RendererUtility& gRendererUtility();
}
