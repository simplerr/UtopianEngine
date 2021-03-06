#include "core/renderer/jobs/OpaqueCopyJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/jobs/DeferredJob.h"

namespace Utopian
{
   OpaqueCopyJob::OpaqueCopyJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   OpaqueCopyJob::~OpaqueCopyJob()
   {
   }

   void OpaqueCopyJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

      opaqueLitImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "Opaque copy HDR image");
      opaqueDepthImage = std::make_shared<Vk::ImageDepth>(mDevice, mWidth, mHeight, VK_FORMAT_D32_SFLOAT_S8_UINT, "Opaque copy depth image");

      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddWriteOnlyColorAttachment(opaqueLitImage);
      mRenderTarget->AddWriteOnlyDepthAttachment(opaqueDepthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/post_process/gbuffer_copy.frag";

      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mEffect->BindCombinedImage("lightSampler", *deferredJob->renderTarget->GetColorImage(), *mRenderTarget->GetSampler());
      mEffect->BindCombinedImage("depthSampler", *gbuffer.depthImage, *mRenderTarget->GetSampler());
   }

   void OpaqueCopyJob::Render(const JobInput& jobInput)
   {
      mRenderTarget->Begin("Opaque copy pass", glm::vec4(0.0, 1.0, 0.5, 1.0));
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
