#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class FresnelJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(FresnelUniforms)
			UNIFORM_PARAM(glm::vec4, eyePos)
		UNIFORM_BLOCK_END()

		FresnelJob(Vk::Device* device, uint32_t width, uint32_t height);
		~FresnelJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

	private:
		SharedPtr<Vk::Effect> mEffect;
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		FresnelUniforms mUniformBlock;
	};
}
