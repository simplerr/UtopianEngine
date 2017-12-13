#pragma once

#include <stdint.h>
#include <glm/glm.hpp>

namespace Vulkan
{
	class Device;
	class FrameBuffers;
	class RenderPass;
	class Sampler;
	class Image;
	class Queue;
	class Pipeline;
	class CommandBuffer;
	class CommandPool;
	class DescriptorSet;

	class RenderTarget
	{
	public:
		RenderTarget(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height);
		~RenderTarget();

		void Begin();
		void End(Vulkan::Queue* queue);

		void SetClearColor(float r, float g, float b, float a = 0.0f);

		Vulkan::Image* GetImage();
		Vulkan::Sampler* GetSampler();
		Vulkan::CommandBuffer* GetCommandBuffer();

		uint32_t GetWidth();
		uint32_t GetHeight();

	private:
		Vulkan::FrameBuffers* mFrameBuffer;
		Vulkan::RenderPass* mRenderPass;
		Vulkan::Image* mColorImage;
		Vulkan::Image* mDepthImage;
		Vulkan::Pipeline* mPipeline;
		Vulkan::CommandBuffer* mCommandBuffer;
		Vulkan::DescriptorSet* mTextureDescriptorSet;
		Vulkan::Sampler* mSampler;
		uint32_t mWidth, mHeight;
		glm::vec4 mClearColor;
	};
}
