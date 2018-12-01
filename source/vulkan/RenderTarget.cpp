#include "vulkan/RenderTarget.h"
#include "vulkan/Device.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/FrameBuffers.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Utopian::Vk
{
	RenderTarget::RenderTarget(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;

		mCommandBuffer = new CommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
		mRenderPass = new RenderPass(device);
		mFrameBuffer = new FrameBuffers(device);
		mSampler = std::make_shared<Sampler>(device);
		
		SetClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	}

	RenderTarget::~RenderTarget()
	{

	}

	void RenderTarget::AddColorAttachment(Image* image, VkImageLayout imageLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
	{
		mRenderPass->AddColorAttachment(image->GetFormat(), imageLayout, loadOp, storeOp);
		mFrameBuffer->AddAttachmentImage(image);
	}

	void RenderTarget::AddDepthAttachment(Image* image, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
	{
		mRenderPass->AddDepthAttachment(image->GetFormat(), loadOp, storeOp);
		mFrameBuffer->AddAttachmentImage(image);
	}

	void RenderTarget::AddColorAttachment(const SharedPtr<Image>& image, VkImageLayout imageLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
	{
		mRenderPass->AddColorAttachment(image->GetFormat(), imageLayout, loadOp, storeOp);
		mFrameBuffer->AddAttachmentImage(image.get());
	}

	void RenderTarget::AddDepthAttachment(const SharedPtr<Image>& image, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
	{
		mRenderPass->AddDepthAttachment(image->GetFormat(), loadOp, storeOp);
		mFrameBuffer->AddAttachmentImage(image.get());
	}

	void RenderTarget::Create()
	{
		mRenderPass->Create();
		mFrameBuffer->Create(mRenderPass, GetWidth(), GetHeight());

		for (uint32_t i = 0; i < mRenderPass->GetNumColorAttachments(); i++)
		{
			VkClearValue clearValue;
			clearValue.color = { mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a };
			mClearValues.push_back(clearValue);
		}

		// Note: Always assumes that the depth stencil attachment is the last one
		VkClearValue clearValue;
		clearValue.depthStencil = { 1.0f, 0 };
		mClearValues.push_back(clearValue);
	}

	void RenderTarget::Begin()
	{
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mRenderPass->GetVkHandle();
		renderPassBeginInfo.renderArea.extent.width = GetWidth();
		renderPassBeginInfo.renderArea.extent.height = GetHeight();
		renderPassBeginInfo.clearValueCount = mClearValues.size();
		renderPassBeginInfo.pClearValues = mClearValues.data();
		renderPassBeginInfo.framebuffer = mFrameBuffer->GetFrameBuffer(0); // TODO: NOTE: Should not be like this

		// Begin command buffer recording & the render pass
		mCommandBuffer->Begin();
		mCommandBuffer->CmdBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		mCommandBuffer->CmdSetViewPort(GetWidth(), GetHeight());
		mCommandBuffer->CmdSetScissor(GetWidth(), GetHeight());
	}

	void RenderTarget::End(Utopian::Vk::Queue* queue)
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

	Utopian::Vk::Sampler* RenderTarget::GetSampler()
	{
		return mSampler.get();
	}

	Utopian::Vk::CommandBuffer* RenderTarget::GetCommandBuffer()
	{
		return mCommandBuffer;
	}

	RenderPass* RenderTarget::GetRenderPass()
	{
		return mRenderPass;
	}

	void RenderTarget::SetClearColor(float r, float g, float b, float a)
	{
		mClearColor = glm::vec4(r, g, b, a);
	}
}