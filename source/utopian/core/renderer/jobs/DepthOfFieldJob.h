#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
   class DepthOfFieldJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(BlurSettings)
         UNIFORM_PARAM(int, size)
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(DOFSettings)
         UNIFORM_PARAM(glm::mat4, projection)
         UNIFORM_PARAM(int, dofEnabled)
         UNIFORM_PARAM(float, dofStart)
         UNIFORM_PARAM(float, dofRange)
      UNIFORM_BLOCK_END()

      DepthOfFieldJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~DepthOfFieldJob();

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

      SharedPtr<Vk::Image> outputImage;
   private:
      void InitBlurPass();
      void InitFocusPass();
      void RenderBlurPass(const JobInput& jobInput);
      void RenderFocusPass(const JobInput& jobInput);
   private:
      struct {
         SharedPtr<Vk::Effect> effect;
         SharedPtr<Vk::RenderTarget> renderTarget;
         SharedPtr<Vk::Image> image;
         BlurSettings settings;
      } mBlur;

      struct {
         SharedPtr<Vk::Effect> effect;
         SharedPtr<Vk::RenderTarget> renderTarget;
         DOFSettings settings;
      } mFocus;

      SharedPtr<Vk::Semaphore> mWaitBlurPassSemaphore;
   };
}
