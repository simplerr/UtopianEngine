#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanSwapChain;

namespace Vulkan
{
	class Device;
	class RenderPass;
	class Image;

	class FrameBuffers
	{
	public:
		// TODO:: Remove this when Image and Texture are fixed
		FrameBuffers(Device* device, RenderPass* renderPass, VkImageView depthView, VkImageView colorView, uint32_t width, uint32_t height);
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
