#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class SunShaftJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(RadialBlurParameters)
			UNIFORM_PARAM(float, radialBlurScale)
			UNIFORM_PARAM(float, radialBlurStrength)
			UNIFORM_PARAM(glm::vec2, radialOrigin)
		UNIFORM_BLOCK_END()

			SunShaftJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~SunShaftJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> radialBlurRenderTarget;
		SharedPtr<Vk::Effect> radialBlurEffect;
	private:
		RadialBlurParameters radialBlurParameters;

		// Todo: Note: This should not be here
		Vk::StaticModel* mSkydomeModel;
		float sunAzimuth;
		glm::vec3 sunDir;
		const float skydomeScale = 1000.0f;
	};
}
