#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/NormalDebugEffect.h"
#include "vulkan/ColorEffect.h"

namespace Utopian
{
	class DebugJob : public BaseJob
	{
	public:
		DebugJob(Vk::Device* device, uint32_t width, uint32_t height);
		~DebugJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::ColorEffect> colorEffect;
		SharedPtr<Vk::ColorEffect> colorEffectWireframe;
		SharedPtr<Vk::NormalDebugEffect> normalEffect;
	private:
		Vk::StaticModel* mCubeModel;
	};
}
