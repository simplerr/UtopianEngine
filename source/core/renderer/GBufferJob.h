#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/GBufferEffect.h"

namespace Utopian
{
	class GBufferJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(GBufferViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SettingsBlock)
			UNIFORM_PARAM(int, normalMapping)
		UNIFORM_BLOCK_END()

			GBufferJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~GBufferJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::Image> positionImage;
		SharedPtr<Vk::Image> normalImage;
		SharedPtr<Vk::Image> normalViewImage; // Normals in view space
		SharedPtr<Vk::Image> albedoImage;
		SharedPtr<Vk::Image> depthImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::GBufferEffect> mGBufferEffect;
		SharedPtr<Vk::GBufferEffect> mGBufferEffectWireframe;
		SharedPtr<Vk::Effect> mGBufferEffectTerrain;
		SharedPtr<Vk::Effect> mGBufferEffectInstanced;
	private:
		GBufferViewProjection viewProjectionBlock;
		SettingsBlock settingsBlock;
		SharedPtr<Vk::Sampler> sampler;
	};
}
