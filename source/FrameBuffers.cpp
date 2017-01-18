#include "FrameBuffers.h"
#include "Device.h"
#include "VulkanDebug.h"
#include "Image.h"
#include "RenderPass.h"
#include "vulkanswapchain.hpp"

namespace VulkanLib
{
	FrameBuffers::FrameBuffers(Device* device, RenderPass* renderPass, Image* depthStencil, VulkanSwapChain* swapChain, uint32_t width, uint32_t height)
		: mDevice(device)
	{
		VkImageView attachments[2];
		attachments[1] = depthStencil->GetView();

		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass->GetVkHandle();
		createInfo.attachmentCount = 2;
		createInfo.pAttachments = attachments;
		createInfo.width = width;
		createInfo.height = height;
		createInfo.layers = 1;

		// Create a frame buffer for each swap chain image
		mFrameBuffers.resize(swapChain->imageCount);
		for (uint32_t i = 0; i < mFrameBuffers.size(); i++)
		{
			attachments[0] = swapChain->buffers[i].view;
			VulkanDebug::ErrorCheck(vkCreateFramebuffer(mDevice->GetVkDevice(), &createInfo, nullptr, &mFrameBuffers[i]));
		}
	}

	FrameBuffers::~FrameBuffers()
	{
		for (uint32_t i = 0; i < mFrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(mDevice->GetVkDevice(), mFrameBuffers[i], nullptr);
		}
	}

	VkFramebuffer FrameBuffers::GetFrameBuffer(uint32_t index)
	{
		// [TODO] Add bound checks
		return mFrameBuffers.at(index);
	}
}