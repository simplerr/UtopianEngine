#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/BlurEffect.h"

namespace Utopian
{
	class TonemapJob : public BaseJob
	{
	public:

		UNIFORM_BLOCK_BEGIN(TonemapSettingsBlock)
			UNIFORM_PARAM(int, tonemapping) // 0 = Reinhard, 1 = Uncharted 2, 2 = Exposure
			UNIFORM_PARAM(float, exposure)
		UNIFORM_BLOCK_END()

		TonemapJob(Vk::Device* device, uint32_t width, uint32_t height);
		~TonemapJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> outputImage;
		SharedPtr<Vk::RenderTarget> renderTarget;
	private:
		SharedPtr<Vk::Effect> effect;
		SharedPtr<Vk::Sampler> sampler;
		TonemapSettingsBlock settingsBlock;
	};
}
