#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/BlurEffect.h"

namespace Utopian
{
	class BlurJob : public BaseJob
	{
	public:
		BlurJob(Vk::Device* device, uint32_t width, uint32_t height);
		~BlurJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> blurImage;
	private:
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		SharedPtr<Vk::BlurEffect> mEffect;
	};
}
