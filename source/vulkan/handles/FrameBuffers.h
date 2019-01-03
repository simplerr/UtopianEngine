#pragma once

#include <vector>
#include "vulkan/VulkanInclude.h"

class VulkanSwapChain;

namespace Utopian::Vk
{
	class FrameBuffers
	{
	public:
		FrameBuffers(Device* device);
		FrameBuffers(Device* device, RenderPass* renderPass, Image* depthStencilImage, Image* colorImage, uint32_t width, uint32_t height);
		FrameBuffers(Device* device, RenderPass* renderPass, Image* depthStencilImage, VulkanSwapChain* swapChain, uint32_t width, uint32_t height);
		~FrameBuffers();

		void AddAttachmentImage(Image* image);
		void AddAttachmentImage(VkImageView imageView);
		void Create(RenderPass* renderPass, uint32_t width, uint32_t height);

		VkFramebuffer GetFrameBuffer(uint32_t index);
		VkFramebuffer GetCurrent();

		uint32_t mCurrentFrameBuffer = 0;
	private:
		std::vector<VkImageView> mAttachments;
		std::vector<VkFramebuffer> mFrameBuffers;
		Device* mDevice;
	};
}
