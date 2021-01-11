#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
	class BloomJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(ExtractSettings)
			UNIFORM_PARAM(float, threshold)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(BlurSettings)
			UNIFORM_PARAM(int, size)
		UNIFORM_BLOCK_END()

		#define OFFSCREEN_RATIO 2u

		BloomJob(Vk::Device* device, uint32_t width, uint32_t height);
		~BloomJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> outputImage;
	private:
		void InitExtractPass();
		void InitBlurPass();
		void RenderExtractPass(const JobInput& jobInput);
		void RenderBlurPass(const JobInput& jobInput);
	private:
		SharedPtr<Vk::Effect> mExtractEffect;
		SharedPtr<Vk::RenderTarget> mExtractRenderTarget;
		SharedPtr<Vk::Image> mBrightColorsImage;

		SharedPtr<Vk::Effect> mBlurEffect;
		SharedPtr<Vk::RenderTarget> mBlurRenderTarget;

		SharedPtr<Vk::Sampler> mSampler;
		SharedPtr<Vk::Semaphore> mWaitExtractPassSemaphore;
		ExtractSettings mExtractSettings;
		BlurSettings mBlurSettings;
	};
}
