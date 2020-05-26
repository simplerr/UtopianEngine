#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class DebugJob : public BaseJob
	{
	public:
		DebugJob(Vk::Device* device, uint32_t width, uint32_t height);
		~DebugJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

	private:
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		SharedPtr<Vk::Effect> mColorEffect;
		SharedPtr<Vk::Effect> mNormalEffect;
	};
}
