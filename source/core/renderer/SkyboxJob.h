#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/SkyboxEffect.h"

namespace Utopian
{
	class SkyboxJob : public BaseJob
	{
	public:
		SkyboxJob(Vk::Device* device, uint32_t width, uint32_t height);
		~SkyboxJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

	private:
		SharedPtr<Vk::CubeMapTexture> mSkybox;
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		SharedPtr<Vk::SkyboxEffect> mEffect;
		Vk::StaticModel* mCubeModel;
	};
}