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

		SSRJob(Vk::Device* device, uint32_t width, uint32_t height);
		~SSRJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

	private:
		void InitFirstPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);
		void InitSecondPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);

		void RenderFirstPass(const JobInput& jobInput);
		void RenderSecondPass(const JobInput& jobInput);

		// Two pass effect
		SharedPtr<Vk::Effect> mCalculateSSREffect;
		SharedPtr<Vk::Effect> mApplySSREffect;
		SharedPtr<Vk::Semaphore> mFirstPassSemaphore;

		SharedPtr<Vk::RenderTarget> mCalculateRenderTarget;
		SharedPtr<Vk::RenderTarget> mApplyRenderTarget;
		SharedPtr<Vk::Image> ssrImage;
		SSRUniforms mUniformBlock;
	};
}
