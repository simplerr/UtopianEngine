#include "vulkan/RenderTarget.h"
#include "vulkan/Device.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/FrameBuffers.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Vulkan
{
	RenderTarget::RenderTarget(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;

		mCommandBuffer = new Vulkan::CommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
		mColorImage = new Vulkan::ImageColor(device, GetWidth(), GetHeight(), VK_FORMAT_R8G8B8A8_UNORM);
		mDepthImage = new Vulkan::ImageDepth(device, GetWidth(), GetHeight(), VK_FORMAT_D32_SFLOAT_S8_UINT);
		mRenderPass = new Vulkan::RenderPass(device, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mFrameBuffer = new Vulkan::FrameBuffers(device, mRenderPass, mDepthImage, mColorImage, GetWidth(), GetHeight());
		mSampler = new Vulkan::Sampler(device);
	}

	RenderTarget::~RenderTarget()
	{

	}

	void RenderTarget::Begin()
	{
		VkClearValue clearValues[2];
		clearValues[0].color = { 0.2f, 0.2f, 0.2f, 0.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mRenderPass->GetVkHandle();
		renderPassBeginInfo.renderArea.extent.width = GetWidth();
		renderPassBeginInfo.renderArea.extent.height = GetHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = mFrameBuffer->GetFrameBuffer(0); // TODO: NOTE: Should not be like this

		// Begin command buffer recording & the render pass
		mCommandBuffer->Begin();
		mCommandBuffer->CmdBeginRenderPass(renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		mCommandBuffer->CmdSetViewPort(GetWidth(), GetHeight());
		mCommandBuffer->CmdSetScissor(GetWidth(), GetHeight());
	}

	void RenderTarget::End(Vulkan::Queue* queue)
	{
		mCommandBuffer->CmdEndRenderPass();
		mCommandBuffer->Flush(queue->GetVkHandle());
	}

	uint32_t RenderTarget::GetWidth()
	{
		return mWidth;
	}

	uint32_t RenderTarget::GetHeight()
	{
		return mHeight;
	}

	Vulkan::Image* RenderTarget::GetImage()
	{
		return mColorImage;
	}

	Vulkan::Sampler* RenderTarget::GetSampler()
	{
		return mSampler;
	}

	Vulkan::CommandBuffer* RenderTarget::GetCommandBuffer()
	{
		return mCommandBuffer;
	}
}