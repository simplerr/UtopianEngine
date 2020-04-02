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

			SSRJob(Vk::Device* device, uint32_t width, uint32_t height);
		~SSRJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

	private:
		void InitTracePass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);
		void InitBlurPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);
		void InitCombinePass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);

		void RenderTracePass(const JobInput& jobInput);
		void RenderBlurPass(const JobInput& jobInput);
		void RenderCombinePass(const JobInput& jobInput);

		// Two pass effect
		SharedPtr<Vk::Effect> mTraceSSREffect;
		SharedPtr<Vk::Effect> mBlurSSREffect;
		SharedPtr<Vk::Effect> mApplySSREffect;
		SharedPtr<Vk::Semaphore> mTracePassSemaphore;
		SharedPtr<Vk::Semaphore> mBlurPassSemaphore;

		SharedPtr<Vk::RenderTarget> mTraceRenderTarget;
		SharedPtr<Vk::RenderTarget> mBlurRenderTarget;
		SharedPtr<Vk::RenderTarget> mApplyRenderTarget;
		SharedPtr<Vk::Image> ssrImage;
		SharedPtr<Vk::Image> ssrBlurImage;
		SSRUniforms mUniformBlock;
	};
}