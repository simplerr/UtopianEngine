#pragma once

#include <stdint.h>
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	class RenderTarget
	{
	public:
		RenderTarget(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height);
		~RenderTarget();

		void Begin();
		
		// Special version that instead of using the framebuffer in RenderTarget
		// will use the supplied one. 
		// Note: Assumes that the renderpass and framebuffers are compatible.
		void Begin(VkFramebuffer framebuffer);
		void End(Utopian::Vk::Queue* queue);

		void SetClearColor(float r, float g, float b, float a = 0.0f);

		/* Note: The order in which these are called is important */
		void AddColorAttachment(Image* image,
								VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void AddDepthAttachment(Image* image,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void AddColorAttachment(const SharedPtr<Image>& image,
		 						VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void AddDepthAttachment(const SharedPtr<Image>& image,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		// Some images can have multiple views (array layers > 1) so in those cases
		// you must provide the exact VkImageView
		void AddColorAttachment(VkImageView imageView,
								VkFormat format,
								VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void AddDepthAttachment(VkImageView imageView,
								VkFormat format,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void Create();

		Utopian::Vk::Sampler* GetSampler();
		Utopian::Vk::CommandBuffer* GetCommandBuffer();
		RenderPass* GetRenderPass();

		uint32_t GetWidth();
		uint32_t GetHeight();

	private:
		FrameBuffers* mFrameBuffer;
		RenderPass* mRenderPass;
		CommandBuffer* mCommandBuffer;
		DescriptorSet* mTextureDescriptorSet;
		SharedPtr<Sampler> mSampler;
		uint32_t mWidth, mHeight;
		glm::vec4 mClearColor;
		std::vector<VkClearValue> mClearValues;
	};
}
