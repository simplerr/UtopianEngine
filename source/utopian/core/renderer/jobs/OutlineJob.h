#pragma once

#include "core/renderer/jobs/BaseJob.h"
#include "vulkan/Effect.h"
#include <vulkan/VulkanPrerequisites.h>

namespace Utopian
{
   class OutlineJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(OutlineSettingsBlock)
         UNIFORM_PARAM(float, outlineWidth)
      UNIFORM_BLOCK_END()

      OutlineJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~OutlineJob();

      void LoadResources() override;
      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

   private:
      void InitMaskPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);
      void InitEdgePass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);

      void RenderMaskPass(const JobInput& jobInput);
      void RenderEdgePass(const JobInput& jobInput);

   private:
      struct {
         SharedPtr<Vk::Effect> effect;
         SharedPtr<Vk::Effect> effectSkinning;
         SharedPtr<Vk::RenderTarget> renderTarget;
         SharedPtr<Vk::Image> image;
         SharedPtr<Vk::Semaphore> semaphore;
      } mMaskPass;

      struct {
         SharedPtr<Vk::Effect> effect;
         SharedPtr<Vk::RenderTarget> renderTarget;
         SharedPtr<Vk::Image> image;
         SharedPtr<Vk::Semaphore> semaphore;
         OutlineSettingsBlock settingsBlock;
      } mEdgePass;
   };
}
