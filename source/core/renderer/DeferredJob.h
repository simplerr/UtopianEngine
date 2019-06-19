#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/DeferredEffect.h"
#include "ScreenQuadRenderer.h"

namespace Utopian
{
	class DeferredJob : public BaseJob
	{
	public:
		DeferredJob(Vk::Device* device, uint32_t width, uint32_t height);
		~DeferredJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::BasicRenderTarget> renderTarget;
	private:
		SharedPtr<Vk::Sampler> mDepthSampler;
		SharedPtr<Vk::DeferredEffect> mEffect;
	};
}
