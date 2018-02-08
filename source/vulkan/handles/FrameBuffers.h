#pragma once

#include <vector>
#include "vulkan/VulkanInclude.h"

class VulkanSwapChain;

namespace Utopian::Vk
{
	class FrameBuffers
	{
	public:
		FrameBuffers(Device* device, RenderPass* renderPass, Image* depthStencilImage, Image* colorImage, uint32_t width, uint32_t height);
		FrameBuffers(Device* device, RenderPass* renderPass, Image* depthStencilImage, VulkanSwapChain* swapChain, uint32_t width, uint32_t height);
		~FrameBuffers();

		VkFramebuffer GetFrameBuffer(uint32_t index);
		VkFramebuffer GetCurrent();

		uint32_t mCurrentFrameBuffer = 0;
	private:
		std::vector<VkFramebuffer> mFrameBuffers;
		Device* mDevice;
	};
}
