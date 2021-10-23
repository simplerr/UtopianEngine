#include "core/renderer/jobs/FresnelJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/jobs/SSRJob.h"
#include "core/renderer/jobs/WaterJob.h"
#include "core/renderer/jobs/OpaqueCopyJob.h"

namespace Utopian
{
   FresnelJob::FresnelJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   FresnelJob::~FresnelJob()
   {
   }

   void FresnelJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      SSRJob* ssrJob = static_cast<SSRJob*>(jobs[JobGraph::SSR_INDEX]);
      OpaqueCopyJob* opaqueCopyJob = static_cast<OpaqueCopyJob*>(jobs[JobGraph::OPAQUE_COPY_INDEX]);
      WaterJob* waterJob = static_cast<WaterJob*>(jobs[JobGraph::WATER_INDEX]);

      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.mainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/post_process/fresnel.frag";
      effectDesc.pipelineDesc.blendingType = Vk::BlendingType::BLENDING_ADDITIVE;

      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mUniformBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mEffect->BindUniformBuffer("UBO_parameters", mUniformBlock);

      mEffect->BindCombinedImage("reflectionSampler", *ssrJob->ssrBlurImage, *mRenderTarget->GetSampler());
      mEffect->BindCombinedImage("refractionSampler", *opaqueCopyJob->opaqueLitImage, *mRenderTarget->GetSampler());
      mEffect->BindCombinedImage("distortionSampler", *waterJob->distortionImage, *mRenderTarget->GetSampler());
      mEffect->BindCombinedImage("positionSampler", *gbuffer.positionImage, *mRenderTarget->GetSampler());
      mEffect->BindCombinedImage("normalSampler", *gbuffer.normalImage, *mRenderTarget->GetSampler());
      mEffect->BindCombinedImage("specularSampler", *gbuffer.specularImage, *mRenderTarget->GetSampler());
   }

   void FresnelJob::Render(const JobInput& jobInput)
   {
      mUniformBlock.data.transparency = jobInput.renderingSettings.waterTransparency;
      mUniformBlock.data.underwaterViewDistance = jobInput.renderingSettings.underwaterViewDistance;
      mUniformBlock.UpdateMemory();

      mRenderTarget->Begin("Fresnel pass", glm::vec4(0.5, 1.0, 0.5, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      if (IsEnabled())
      {
         commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mEffect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
