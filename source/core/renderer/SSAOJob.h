#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/SSAOEffect.h"

namespace Utopian
{
	class SSAOJob : public BaseJob
	{
	public:
		SSAOJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~SSAOJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::Image> ssaoImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::SSAOEffect> effect;
	private:
		void CreateKernelSamples();
	};
}
