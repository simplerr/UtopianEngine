#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	/* Renders copies of the deferred job output images needed by other jobs, transparency for example. */
	class GeometryThicknessJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_BLOCK_END()

		GeometryThicknessJob(Vk::Device* device, uint32_t width, uint32_t height);
		~GeometryThicknessJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> geometryThicknessImage;
	private:
		SharedPtr<Vk::Effect> mEffect;
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		ViewProjection mViewProjectionBlock;

		const float DEFAULT_THICKNESS = 1.0f;
	};
}
