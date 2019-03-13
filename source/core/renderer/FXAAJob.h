#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/BlurEffect.h"

namespace Utopian
{
	class FXAAJob : public BaseJob
	{
	public:
		FXAAJob(Vk::Device* device, uint32_t width, uint32_t height);
		~FXAAJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> fxaaImage;
		SharedPtr<Vk::RenderTarget> renderTarget;
	private:
		SharedPtr<Vk::Effect> effect;
		SharedPtr<Vk::Sampler> sampler;
	};
}
