#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class PixelDebugJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(MouseInput)
			UNIFORM_PARAM(glm::vec2, mousePosUV)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(OutputBuffer)
			UNIFORM_PARAM(glm::vec4, pixelValue)
		UNIFORM_BLOCK_END()

		PixelDebugJob(Vk::Device* device, uint32_t width, uint32_t height);
		~PixelDebugJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;
		void Update() override;

	private:
		SharedPtr<Vk::Effect> mEffect;
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		MouseInput mMouseInputBlock;
		OutputBuffer mOutputBuffer;
	};
}
