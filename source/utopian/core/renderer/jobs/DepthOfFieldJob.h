#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
   class DepthOfFieldJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(BlurSettings)
         UNIFORM_PARAM(float, blurScale)
         UNIFORM_PARAM(float, blurStrength)
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
      void InitBlurPasses();
      void InitDilatePass();
      void InitFocusPass();
      void RenderHorizontalBlurPass(const JobInput& jobInput);
      void RenderVerticalBlurPass(const JobInput& jobInput);
      void RenderDilatePass(const JobInput& jobInput);
      void RenderFocusPass(const JobInput& jobInput);
   private:
      struct {
         SharedPtr<Vk::Effect> horizontalEffect;
         SharedPtr<Vk::Effect> combinedEffect;
         SharedPtr<Vk::RenderTarget> horizontalRenderTarget;
         SharedPtr<Vk::RenderTarget> combinedRenderTarget;
         SharedPtr<Vk::Image> horizontalImage;
         SharedPtr<Vk::Image> combinedImage;
         BlurSettings settings;
      } mBlur;

      struct {
         SharedPtr<Vk::Effect> effect;
         SharedPtr<Vk::RenderTarget> renderTarget;
         SharedPtr<Vk::Image> image;
      } mDilate;

      struct {
         SharedPtr<Vk::Effect> effect;
         SharedPtr<Vk::RenderTarget> renderTarget;
         DOFSettings settings;
      } mFocus;

      SharedPtr<Vk::Semaphore> mWaitHorizontalBlurPassSemaphore;
      SharedPtr<Vk::Semaphore> mWaitVerticalBlurPassSemaphore;
      SharedPtr<Vk::Semaphore> mWaitDilatePassSemaphore;
   };
}
