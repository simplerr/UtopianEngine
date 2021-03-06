#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
   class FXAAJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(FXAASettingsBlock)
         UNIFORM_PARAM(int, enabled)
         UNIFORM_PARAM(int, debug)
         UNIFORM_PARAM(float, threshold)
      UNIFORM_BLOCK_END()

      FXAAJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~FXAAJob();

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

   private:
      SharedPtr<Vk::Image> mFXXAImage;
      SharedPtr<Vk::RenderTarget> mRenderTarget;
      SharedPtr<Vk::Effect> mEffect;
      SharedPtr<Vk::Sampler> mSampler;
      FXAASettingsBlock mSettingsBlock;
   };
}
