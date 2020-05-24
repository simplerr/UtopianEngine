#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian
{
	class BlurJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(BlurSettingsBlock)
			UNIFORM_PARAM(int, blurRange)
		UNIFORM_BLOCK_END()

		BlurJob(Vk::Device* device, uint32_t width, uint32_t height);
		~BlurJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> blurImage;
	private:
		SharedPtr<Vk::RenderTarget> mRenderTarget;
		SharedPtr<Vk::Effect> mEffect;
		BlurSettingsBlock settingsBlock;
	};
}
