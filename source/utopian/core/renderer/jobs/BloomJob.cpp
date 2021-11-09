#include "core/renderer/jobs/BloomJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"

namespace Utopian
{
   BloomJob::BloomJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      //mWidth = mWidth / 16.0;
      //mHeight = mHeight / 16.0;
      InitExtractPass();
      InitBlurPass();

      mWaitExtractPassSemaphore = std::make_shared<Vk::Semaphore>(mDevice);
   }

   BloomJob::~BloomJob()
   {
   }

   void BloomJob::LoadResources()
   {
      auto loadShaders = [&]()
      {
         Vk::EffectCreateInfo effectDescExtract;
         effectDescExtract.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
         effectDescExtract.shaderDesc.fragmentShaderPath = "data/shaders/post_process/bloom_extract.frag";
         mExtractEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mExtractRenderTarget->GetRenderPass(), effectDescExtract);

         Vk::EffectCreateInfo effectDescBlur;
         effectDescBlur.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
         effectDescBlur.shaderDesc.fragmentShaderPath = "data/shaders/post_process/bloom_blur.frag";
         mBlurEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mBlurRenderTarget->GetRenderPass(), effectDescBlur);
      };

      loadShaders();
   }

   void BloomJob::InitExtractPass()
   {
      mBrightColorsImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth / OFFSCREEN_RATIO, mHeight / OFFSCREEN_RATIO, VK_FORMAT_R16G16B16A16_SFLOAT, "Bloom bright image");

      mExtractRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth / OFFSCREEN_RATIO, mHeight / OFFSCREEN_RATIO);
      mExtractRenderTarget->AddWriteOnlyColorAttachment(mBrightColorsImage);
      mExtractRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      mExtractRenderTarget->Create();

      //const uint32_t size = 240;
      //gScreenQuadUi().AddQuad(5 * (size + 10) + 10, mHeight - (size + 10), size, size, brightColorsImage.get(), extractRenderTarget->GetSampler());
   }

   void BloomJob::InitBlurPass()
   {
      outputImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth / OFFSCREEN_RATIO, mHeight / OFFSCREEN_RATIO, VK_FORMAT_R16G16B16A16_SFLOAT, "Bloom output image");

      mBlurRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth / OFFSCREEN_RATIO, mHeight / OFFSCREEN_RATIO);
      mBlurRenderTarget->AddWriteOnlyColorAttachment(outputImage);
      mBlurRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      mBlurRenderTarget->Create();

      /*const uint32_t size = 240;
      gScreenQuadUi().AddQuad(5 * (size + 10) + 10, mHeight - (2 * size + 10), size, size, outputImage.get(), blurRenderTarget->GetSampler());*/
   }

   void BloomJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
   }

   void BloomJob::PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mExtractSettings.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mExtractEffect->BindUniformBuffer("UBO_settings", mExtractSettings);
      mExtractEffect->BindCombinedImage("hdrSampler", *gbuffer.mainImage, *mExtractRenderTarget->GetSampler());

      mBlurSettings.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mBlurEffect->BindUniformBuffer("UBO_settings", mBlurSettings);
      mBlurEffect->BindCombinedImage("hdrSampler", *mBrightColorsImage, *mBlurRenderTarget->GetSampler());
   }

   void BloomJob::RenderExtractPass(const JobInput& jobInput)
   {
      mExtractSettings.data.threshold = jobInput.renderingSettings.bloomThreshold;
      mExtractSettings.UpdateMemory();

      mExtractRenderTarget->Begin("Bloom extract pass");

      if (IsEnabled())
      {
         Vk::CommandBuffer* commandBuffer = mExtractRenderTarget->GetCommandBuffer();
         commandBuffer->CmdBindPipeline(mExtractEffect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mExtractEffect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mExtractRenderTarget->End(GetWaitSemahore(), mWaitExtractPassSemaphore);
   }

   void BloomJob::RenderBlurPass(const JobInput& jobInput)
   {
      mBlurSettings.data.size = 5;// = jobInput.renderingSettings.bloomThreshold;
      mBlurSettings.UpdateMemory();

      mBlurRenderTarget->Begin("Bloom blur pass", glm::vec4(0.1f, 0.5f, 0.9f, 1.0f));

      if (IsEnabled())
      {
         Vk::CommandBuffer* commandBuffer = mBlurRenderTarget->GetCommandBuffer();
         commandBuffer->CmdBindPipeline(mBlurEffect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mBlurEffect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mBlurRenderTarget->End(mWaitExtractPassSemaphore, GetCompletedSemahore());
   }

   void BloomJob::Render(const JobInput& jobInput)
   {
      RenderExtractPass(jobInput);
      RenderBlurPass(jobInput);
   }
}
