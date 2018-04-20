#pragma once

#include <stdint.h>
#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class RenderTarget
	{
	public:
		RenderTarget(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height);
		~RenderTarget();

		void Begin();
		void End(Utopian::Vk::Queue* queue);

		void SetClearColor(float r, float g, float b, float a = 0.0f);

		/* Note: The order in which these are called is important */
		void AddColorAttachment(Image* image, VkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		void AddDepthAttachment(Image* image);

		void Create();

		Utopian::Vk::Sampler* GetSampler();
		Utopian::Vk::CommandBuffer* GetCommandBuffer();
		RenderPass* GetRenderPass();

		uint32_t GetWidth();
		uint32_t GetHeight();

	private:
		Utopian::Vk::FrameBuffers* mFrameBuffer;
		Utopian::Vk::RenderPass* mRenderPass;
		Utopian::Vk::Pipeline* mPipeline;
		Utopian::Vk::CommandBuffer* mCommandBuffer;
		Utopian::Vk::DescriptorSet* mTextureDescriptorSet;
		Utopian::Vk::Sampler* mSampler;
		uint32_t mWidth, mHeight;
		glm::vec4 mClearColor;
	};
}
