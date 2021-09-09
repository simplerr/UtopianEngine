#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
   class Model;

   class SunShaftJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(RadialBlurParameters)
         UNIFORM_PARAM(float, radialBlurScale)
         UNIFORM_PARAM(float, radialBlurStrength)
         UNIFORM_PARAM(glm::vec2, radialOrigin)
      UNIFORM_BLOCK_END()

      SunShaftJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~SunShaftJob();

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

   private:
      SharedPtr<Vk::RenderTarget> mRadialBlurRenderTarget;
      SharedPtr<Vk::Effect> mRadialBlurEffect;
      RadialBlurParameters mRadialBlurParameters;

      // Todo: Note: This should not be here
      SharedPtr<Model> mSkydomeModel;
      const float mSkydomeScale = 1000.0f;
   };
}
