#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
   /* Renders copies of the deferred job output images needed by other jobs, transparency for example. */
   class OpaqueCopyJob : public BaseJob
   {
   public:
      OpaqueCopyJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~OpaqueCopyJob();

      void LoadResources() override;

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

      SharedPtr<Vk::Image> opaqueLitImage;
      SharedPtr<Vk::Image> opaqueDepthImage;
   private:
      SharedPtr<Vk::Effect> mEffect;
      SharedPtr<Vk::RenderTarget> mRenderTarget;
   };
}
