#include "core/renderer/jobs/DebugJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"

namespace Utopian
{
   DebugJob::DebugJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   DebugJob::~DebugJob()
   {
   }

   void DebugJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      //renderTarget->AddDepthAttachment(gbuffer.depthImage, VK_ATTACHMENT_LOAD_OP_LOAD);
      mRenderTarget->SetClearColor(1, 1, 1, 1);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/color/color.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/color/color.frag";
      mColorEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/normal_debug/normal_debug.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/normal_debug/normal_debug.frag";
      effectDesc.shaderDesc.geometryShaderPath = "data/shaders/normal_debug/normal_debug.geom";
      mNormalEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mColorEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mNormalEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
   }

   void DebugJob::Render(const JobInput& jobInput)
   {
      mRenderTarget->Begin("Debug pass", glm::vec4(0.3, 0.6, 0.9, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      /* Render all renderables */
      for (auto& renderable : jobInput.sceneInfo.renderables)
      {
         if (!renderable->IsVisible())
            continue;

         Vk::StaticModel* model = renderable->GetModel();

         if (renderable->HasRenderFlags(RENDER_FLAG_COLOR))
         {
            for (Primitive* mesh : model->mMeshes)
            {
               // Push the world matrix constant
               Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor());

               commandBuffer->CmdBindPipeline(mColorEffect->GetPipeline());
               commandBuffer->CmdBindDescriptorSets(mColorEffect);
               commandBuffer->CmdPushConstants(mColorEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);
               commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
               commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
               commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
            }
         }

         if (renderable->HasRenderFlags(RENDER_FLAG_NORMAL_DEBUG))
         {
            for (Primitive* mesh : model->mMeshes)
            {
               // Push the world matrix constant
               Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix());

               commandBuffer->CmdBindPipeline(mNormalEffect->GetPipeline());
               commandBuffer->CmdBindDescriptorSets(mNormalEffect);
               commandBuffer->CmdPushConstants(mNormalEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);
               commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
               commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
               commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
            }
         }
      }

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
