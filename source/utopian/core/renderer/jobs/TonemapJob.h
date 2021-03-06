#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
   class TonemapJob : public BaseJob
   {
   public:

      UNIFORM_BLOCK_BEGIN(TonemapSettingsBlock)
         UNIFORM_PARAM(int, tonemapping) // 0 = Reinhard, 1 = Uncharted 2, 2 = Exposure
         UNIFORM_PARAM(float, exposure)
      UNIFORM_BLOCK_END()

      TonemapJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~TonemapJob();

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

      SharedPtr<Vk::Image> outputImage;
   private:
      SharedPtr<Vk::RenderTarget> mRenderTarget;
      SharedPtr<Vk::Effect> mEffect;
      SharedPtr<Vk::Sampler> mSampler;
      TonemapSettingsBlock mSettingsBlock;
   };
}
