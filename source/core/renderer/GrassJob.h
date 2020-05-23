#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class GrassJob : public BaseJob
	{
	public:
		struct GrassInstance
		{
			glm::vec4 position;
			glm::vec3 color;
			int textureIndex;
			//glm::vec3 scale;
		};

		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_PARAM(glm::vec4, eyePos)
			UNIFORM_PARAM(float, grassViewDistance)
		UNIFORM_BLOCK_END()

		GrassJob(Vk::Device* device, uint32_t width, uint32_t height);
		~GrassJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

	private:
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		SharedPtr<Vk::Effect> mEffect;
		SharedPtr<Vk::Sampler> mSampler;
		ViewProjection mViewProjectionBlock;
	};
}
