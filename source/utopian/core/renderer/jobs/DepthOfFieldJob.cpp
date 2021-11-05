#include "core/renderer/jobs/DepthOfFieldJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"

namespace Utopian
{
   DepthOfFieldJob::DepthOfFieldJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      InitBlurPass();
      InitFocusPass();

      mWaitBlurPassSemaphore = std::make_shared<Vk::Semaphore>(mDevice);
   }

   DepthOfFieldJob::~DepthOfFieldJob()
   {
   }

   void DepthOfFieldJob::InitBlurPass()
   {
      mBlur.image = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "DOF blur image");

      mBlur.renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mBlur.renderTarget->AddWriteOnlyColorAttachment(mBlur.image);
      mBlur.renderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      mBlur.renderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/post_process/bloom_blur.frag";

      mBlur.effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mBlur.renderTarget->GetRenderPass(), effectDesc);

      mBlur.settings.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mBlur.effect->BindUniformBuffer("UBO_settings", mBlur.settings);
   }

   void DepthOfFieldJob::InitFocusPass()
   {
      outputImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "DOF output");

      mFocus.renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mFocus.renderTarget->AddWriteOnlyColorAttachment(outputImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mFocus.renderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      mFocus.renderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/dof/dof.frag";

      mFocus.effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mFocus.renderTarget->GetRenderPass(), effectDesc);

      mFocus.settings.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mFocus.effect->BindUniformBuffer("UBO_settings", mFocus.settings);
   }

   void DepthOfFieldJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mBlur.effect->BindCombinedImage("hdrSampler", *gbuffer.mainImage, *mBlur.renderTarget->GetSampler());

      mFocus.effect->BindCombinedImage("normalTexture", *gbuffer.mainImage, *mFocus.renderTarget->GetSampler());
      mFocus.effect->BindCombinedImage("blurredTexture", *mBlur.image, *mFocus.renderTarget->GetSampler());
      mFocus.effect->BindCombinedImage("depthTexture", *gbuffer.depthImage, *mFocus.renderTarget->GetSampler());
   }

   void DepthOfFieldJob::RenderBlurPass(const JobInput& jobInput)
   {
      mBlur.settings.data.size = 5;
      mBlur.settings.UpdateMemory();

      mBlur.renderTarget->Begin("DOF blur pass", glm::vec4(0.1f, 0.5f, 0.9f, 1.0f));

      if (IsEnabled())
      {
         Vk::CommandBuffer* commandBuffer = mBlur.renderTarget->GetCommandBuffer();
         commandBuffer->CmdBindPipeline(mBlur.effect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mBlur.effect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mBlur.renderTarget->End(GetWaitSemahore(), mWaitBlurPassSemaphore);
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

      mFocus.renderTarget->End(mWaitBlurPassSemaphore, GetCompletedSemahore());
   }

   void DepthOfFieldJob::Render(const JobInput& jobInput)
   {
      RenderBlurPass(jobInput);
      RenderFocusPass(jobInput);
   }
}
