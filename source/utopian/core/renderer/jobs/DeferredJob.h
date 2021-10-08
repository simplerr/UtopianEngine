#pragma once

#include "core/renderer/jobs/BaseJob.h"
#include "core/renderer/jobs/AtmosphereJob.h"
#include "vulkan/VulkanPrerequisites.h"
#include "core/renderer/CommonBuffers.h"

namespace Utopian
{
   class DeferredJob : public BaseJob
   {
   public:
      DeferredJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~DeferredJob();

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

      SharedPtr<Vk::BasicRenderTarget> renderTarget;
   private:
      SharedPtr<Vk::Sampler> mDepthSampler;
      SharedPtr<Vk::Effect> mPhongEffect;
      SharedPtr<Vk::Effect> mPbrEffect;
      SharedPtr<Vk::Sampler> mSampler;
      LightUniformBuffer light_ubo;
      SettingsUniformBuffer settings_ubo;
      CascadeBlock cascade_ubo;
      AtmosphereJob::ParameterBlock atmosphere_ubo;
   };
}
