#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class SkydomeJob : public BaseJob
	{
	public:

		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_PARAM(glm::mat4, world)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(ParameterBlock)
			UNIFORM_PARAM(float, sphereRadius)
			UNIFORM_PARAM(float, inclination)
			UNIFORM_PARAM(float, azimuth)
			UNIFORM_PARAM(float, time)
			UNIFORM_PARAM(float, sunSpeed)
			UNIFORM_PARAM(int, onlySun)
		UNIFORM_BLOCK_END()

			SkydomeJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~SkydomeJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::Effect> effect;
		SharedPtr<Vk::Image> sunImage;
	private:
		ViewProjection viewProjectionBlock;
		ParameterBlock parameterBlock;
		Vk::StaticModel* mSkydomeModel;
		float sunAzimuth;
	};
}
