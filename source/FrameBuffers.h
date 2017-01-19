#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanSwapChain;

namespace VulkanLib
{
	class Device;
	class RenderPass;
	class Image;

	class FrameBuffers
	{
	public:
		FrameBuffers(Device* device, RenderPass* renderPass, Image* depthStencil, VulkanSwapChain* swapChain, uint32_t width, uint32_t height);
		~FrameBuffers();

		VkFramebuffer GetFrameBuffer(uint32_t index);
		VkFramebuffer GetCurrent();

		uint32_t mCurrentFrameBuffer = 0;
	private:
		std::vector<VkFramebuffer> mFrameBuffers;
		Device* mDevice;
	};
}
