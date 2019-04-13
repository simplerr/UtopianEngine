#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class DebugJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_BLOCK_END()

		DebugJob(Vk::Device* device, uint32_t width, uint32_t height);
		~DebugJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::Effect> colorEffect;
		SharedPtr<Vk::Effect> normalEffect;
	private:
		ViewProjection viewProjectionBlock;
	};
}
