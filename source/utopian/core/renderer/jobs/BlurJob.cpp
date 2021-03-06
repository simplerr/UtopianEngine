#include "core/renderer/jobs/BlurJob.h"
#include "core/renderer/jobs/SSAOJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/EffectManager.h"

namespace Utopian
{
   BlurJob::BlurJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      blurImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "Blur image");

      mRenderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
      mRenderTarget->AddWriteOnlyColorAttachment(blurImage);
      mRenderTarget->SetClearColor(1, 1, 1, 1);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/blur/blur.frag";

      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, mRenderTarget->GetRenderPass(), effectDesc);

      /*const uint32_t size = 240;
      gScreenQuadUi().AddQuad(10, height - (size + 10), size, size, blurImage.get(), renderTarget->GetSampler());*/
   }

   BlurJob::~BlurJob()
   {
   }

   void BlurJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      SSAOJob* ssaoJob = static_cast<SSAOJob*>(jobs[JobGraph::SSAO_INDEX]);

      settingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      mEffect->BindUniformBuffer("UBO_settings", settingsBlock);

      mEffect->BindCombinedImage("samplerSSAO", *ssaoJob->ssaoImage, *ssaoJob->renderTarget->GetSampler());
   }

   void BlurJob::Render(const JobInput& jobInput)
   {
      settingsBlock.data.blurRange = jobInput.renderingSettings.blurRadius;
      settingsBlock.UpdateMemory();

      mRenderTarget->Begin("SSAO blur pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      if (IsEnabled())
      {
         // Todo: Should this be moved to the effect instead?
         commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mEffect);

         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
