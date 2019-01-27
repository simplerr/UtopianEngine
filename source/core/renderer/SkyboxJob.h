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

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::CubeMapTexture> skybox;
		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::SkyboxEffect> effect;
	private:
		Vk::StaticModel* mCubeModel;
	};
}