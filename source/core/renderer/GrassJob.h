#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class GrassJob : public BaseJob
	{
	public:

		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_PARAM(glm::vec4, eyePos)
			UNIFORM_PARAM(float, grassViewDistance)
		UNIFORM_BLOCK_END()

		GrassJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~GrassJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;
		SharedPtr<Vk::Effect> effect;
		SharedPtr<Vk::Sampler> sampler;
		ViewProjection viewProjectionBlock;
	private:
	};
}
