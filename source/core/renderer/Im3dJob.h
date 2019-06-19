#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class Im3dJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(ViewportBlock)
			UNIFORM_PARAM(glm::vec2, viewport)
		UNIFORM_BLOCK_END()

		Im3dJob(Vk::Device* device, uint32_t width, uint32_t height);
		~Im3dJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

	private:
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		SharedPtr<Vk::Effect> mLinesEffect;
		SharedPtr<Vk::Effect> mPointsEffect;
		SharedPtr<Vk::Effect> mTrianglesEffect;
		SharedPtr<Vk::VertexDescription> mVertexDescription;

		ViewProjection mViewProjectionBlock;
		ViewportBlock mViewportBlock;
	};
}
