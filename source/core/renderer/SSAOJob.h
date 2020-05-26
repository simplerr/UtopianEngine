#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/Effect.h"

namespace Utopian
{
	class SSAOJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(KernelSampleBlock)
			UNIFORM_PARAM(glm::vec4, samples[64]) // Todo: Move to it's own block so the rest can be reused
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SSAOSettingsBlock)
			UNIFORM_PARAM(float, radius)
			UNIFORM_PARAM(float, bias)
		UNIFORM_BLOCK_END()

		SSAOJob(Vk::Device* device, uint32_t width, uint32_t height);
		~SSAOJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> ssaoImage;
		SharedPtr<Vk::RenderTarget> renderTarget;
	private:
		void CreateKernelSamples();

		SharedPtr<Vk::Effect> mEffect;
		KernelSampleBlock mKernelSampleBlock;
		SSAOSettingsBlock settingsBlock;
	};
}
