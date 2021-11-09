#include "core/renderer/jobs/DepthOfFieldJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"
#include "core/Log.h"
#include <vulkan/vulkan_core.h>
#include <thread>

namespace Utopian
{
   DepthOfFieldJob::DepthOfFieldJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      InitBlurPasses();
      InitDilatePass();
      InitFocusPass();

      mWaitHorizontalBlurPassSemaphore = std::make_shared<Vk::Semaphore>(mDevice);
      mWaitVerticalBlurPassSemaphore = std::make_shared<Vk::Semaphore>(mDevice);
      mWaitDilatePassSemaphore = std::make_shared<Vk::Semaphore>(mDevice);
   }

   DepthOfFieldJob::~DepthOfFieldJob()
   {
   }

   void DepthOfFieldJob::LoadResources()
   {
      auto loadShaders = [&]()
      {
         Vk::EffectCreateInfo effectDesc;
         effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
         effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/post_process/gaussian_blur.frag";
         mBlur.horizontalEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mBlur.horizontalRenderTarget->GetRenderPass(), effectDesc);

         Vk::EffectCreateInfo effectDescCombined;
         effectDescCombined.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
         effectDescCombined.shaderDesc.fragmentShaderPath = "data/shaders/post_process/gaussian_blur.frag";
         mBlur.combinedEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mBlur.combinedRenderTarget->GetRenderPass(), effectDescCombined);

         Vk::EffectCreateInfo effectDescDilate;
         effectDescDilate.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
         effectDescDilate.shaderDesc.fragmentShaderPath = "data/shaders/post_process/dilate.frag";
         mDilate.effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mDilate.renderTarget->GetRenderPass(), effectDescDilate);

         Vk::EffectCreateInfo effectDescFocus;
         effectDescFocus.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
         effectDescFocus.shaderDesc.fragmentShaderPath = "data/shaders/dof/dof.frag";
         mFocus.effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mFocus.renderTarget->GetRenderPass(), effectDescFocus);
      };

      loadShaders();
   }

   void DepthOfFieldJob::InitBlurPasses()
   {
      mBlur.horizontalImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "DOF horizontal blur image");
      mBlur.combinedImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "DOF combined blur image");

      mBlur.horizontalRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mBlur.horizontalRenderTarget->AddWriteOnlyColorAttachment(mBlur.horizontalImage);
      mBlur.horizontalRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      mBlur.horizontalRenderTarget->Create();

      mBlur.combinedRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mBlur.combinedRenderTarget->AddWriteOnlyColorAttachment(mBlur.combinedImage);
      mBlur.combinedRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      mBlur.combinedRenderTarget->Create();
   }

   void DepthOfFieldJob::InitDilatePass()
   {
      mDilate.image = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "DOF dilate output");

      mDilate.renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mDilate.renderTarget->AddWriteOnlyColorAttachment(mDilate.image);
      mDilate.renderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      mDilate.renderTarget->Create();
   }

   void DepthOfFieldJob::InitFocusPass()
   {
      outputImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "DOF output");

      mFocus.renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mFocus.renderTarget->AddWriteOnlyColorAttachment(outputImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mFocus.renderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      mFocus.renderTarget->Create();
   }

   void DepthOfFieldJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
   }

   void DepthOfFieldJob::PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mBlur.settings.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mBlur.horizontalEffect->BindUniformBuffer("UBO_settings", mBlur.settings);
      mBlur.combinedEffect->BindUniformBuffer("UBO_settings", mBlur.settings);
      mBlur.horizontalEffect->BindCombinedImage("hdrSampler", *gbuffer.mainImage, *mBlur.horizontalRenderTarget->GetSampler());
      mBlur.combinedEffect->BindCombinedImage("hdrSampler", *mBlur.horizontalImage, *mBlur.combinedRenderTarget->GetSampler());

      mDilate.effect->BindCombinedImage("inputTexture", *mBlur.combinedImage, *mDilate.renderTarget->GetSampler());

      mFocus.settings.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mFocus.effect->BindUniformBuffer("UBO_settings", mFocus.settings);
      mFocus.effect->BindCombinedImage("normalTexture", *gbuffer.mainImage, *mFocus.renderTarget->GetSampler());
      mFocus.effect->BindCombinedImage("blurredTexture", *mDilate.image, *mFocus.renderTarget->GetSampler());
      mFocus.effect->BindCombinedImage("depthTexture", *gbuffer.depthImage, *mFocus.renderTarget->GetSampler());
   }

   void DepthOfFieldJob::RenderHorizontalBlurPass(const JobInput& jobInput)
   {
      mBlur.settings.data.blurScale = 1.0f;
      mBlur.settings.data.blurStrength = 1.0f;
      mBlur.settings.UpdateMemory();

      mBlur.horizontalRenderTarget->Begin("DOF horizontal blur pass", glm::vec4(0.1f, 0.5f, 0.9f, 1.0f));

      if (IsEnabled())
      {
         int direction = 0; // Horizontal
         Vk::CommandBuffer* commandBuffer = mBlur.horizontalRenderTarget->GetCommandBuffer();
         commandBuffer->CmdBindPipeline(mBlur.horizontalEffect->GetPipeline());
         commandBuffer->CmdPushConstants(mBlur.horizontalEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(int), &direction);
         commandBuffer->CmdBindDescriptorSets(mBlur.horizontalEffect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mBlur.horizontalRenderTarget->End(GetWaitSemahore(), mWaitHorizontalBlurPassSemaphore);
   }

   void DepthOfFieldJob::RenderVerticalBlurPass(const JobInput& jobInput)
   {
      mBlur.combinedRenderTarget->Begin("DOF vertical blur pass", glm::vec4(0.1f, 0.5f, 0.9f, 1.0f));

      if (IsEnabled())
      {
         int direction = 1; // Vertical
         Vk::CommandBuffer* commandBuffer = mBlur.combinedRenderTarget->GetCommandBuffer();
         commandBuffer->CmdBindPipeline(mBlur.combinedEffect->GetPipeline());
         commandBuffer->CmdPushConstants(mBlur.combinedEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(int), &direction);
         commandBuffer->CmdBindDescriptorSets(mBlur.combinedEffect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mBlur.combinedRenderTarget->End(mWaitHorizontalBlurPassSemaphore, mWaitVerticalBlurPassSemaphore);
   }

   void DepthOfFieldJob::RenderDilatePass(const JobInput& jobInput)
   {
      mDilate.renderTarget->Begin("DOF dilate pass", glm::vec4(0.1f, 0.0f, 0.3f, 1.0f));

      if (IsEnabled())
      {
         Vk::CommandBuffer* commandBuffer = mDilate.renderTarget->GetCommandBuffer();
         commandBuffer->CmdBindPipeline(mDilate.effect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mDilate.effect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mDilate.renderTarget->End(mWaitVerticalBlurPassSemaphore, mWaitDilatePassSemaphore);
   }

   void DepthOfFieldJob::RenderFocusPass(const JobInput& jobInput)
   {
      mFocus.settings.data.projection = jobInput.sceneInfo.sharedVariables.data.projectionMatrix;
      mFocus.settings.data.dofEnabled = jobInput.renderingSettings.dofEnabled;
      mFocus.settings.data.dofStart = jobInput.renderingSettings.dofStart;
      mFocus.settings.data.dofRange = jobInput.renderingSettings.dofRange;
      mFocus.settings.UpdateMemory();
      mFocus.renderTarget->Begin("DOF Focus pass");

      Vk::CommandBuffer* commandBuffer = mFocus.renderTarget->GetCommandBuffer();
      commandBuffer->CmdBindPipeline(mFocus.effect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(mFocus.effect);
      gRendererUtility().DrawFullscreenQuad(commandBuffer);

      mFocus.renderTarget->End(mWaitDilatePassSemaphore, GetCompletedSemahore());
   }

   void DepthOfFieldJob::Render(const JobInput& jobInput)
   {
      RenderHorizontalBlurPass(jobInput);
      RenderVerticalBlurPass(jobInput);
      RenderDilatePass(jobInput);
      RenderFocusPass(jobInput);
   }
}
