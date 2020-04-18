#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/SSAOEffect.h"

namespace Utopian
{
	class SSRJob : public BaseJob
	{
	public:

		UNIFORM_BLOCK_BEGIN(SSRUniforms)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_PARAM(glm::mat4, projection)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(BlurSettingsBlock)
			UNIFORM_PARAM(int, blurRange)
		UNIFORM_BLOCK_END()

		// Note: Todo: This should be shared with SkydomeJob
        UNIFORM_BLOCK_BEGIN(SkyParameterBlock)
			UNIFORM_PARAM(glm::vec3, eyePos)
			UNIFORM_PARAM(float, sphereRadius)
			UNIFORM_PARAM(float, inclination)
			UNIFORM_PARAM(float, azimuth)
			UNIFORM_PARAM(float, time)
			UNIFORM_PARAM(float, sunSpeed)
			UNIFORM_PARAM(int, onlySun)
		UNIFORM_BLOCK_END()

		SSRJob(Vk::Device* device, uint32_t width, uint32_t height);
		~SSRJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> ssrBlurImage;

	private:
		void InitTracePass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);
		void InitBlurPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);

		void RenderTracePass(const JobInput& jobInput);
		void RenderBlurPass(const JobInput& jobInput);

		// Two pass effect
		SharedPtr<Vk::Effect> mTraceSSREffect;
		SharedPtr<Vk::Effect> mBlurSSREffect;
		SharedPtr<Vk::Semaphore> mTracePassSemaphore;
		SharedPtr<Vk::Semaphore> mBlurPassSemaphore;

		SharedPtr<Vk::RenderTarget> mTraceRenderTarget;
		SharedPtr<Vk::RenderTarget> mBlurRenderTarget;
		SharedPtr<Vk::Image> ssrImage;
		SSRUniforms mUniformBlock;
		SkyParameterBlock mSkyParameterBlock;
	};
}