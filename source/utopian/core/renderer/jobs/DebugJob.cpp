#include "core/renderer/jobs/DebugJob.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Model.h"

namespace Utopian
{
   DebugJob::DebugJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   DebugJob::~DebugJob()
   {
   }

   void DebugJob::LoadResources()
   {
      auto loadShaders = [&]()
      {
         Vk::EffectCreateInfo effectDescColor;
         effectDescColor.shaderDesc.vertexShaderPath = "data/shaders/color/color.vert";
         effectDescColor.shaderDesc.fragmentShaderPath = "data/shaders/color/color.frag";
         mColorEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDescColor);

         Vk::EffectCreateInfo effectDescNormal;
         effectDescNormal.shaderDesc.vertexShaderPath = "data/shaders/normal_debug/normal_debug.vert";
         effectDescNormal.shaderDesc.fragmentShaderPath = "data/shaders/normal_debug/normal_debug.frag";
         effectDescNormal.shaderDesc.geometryShaderPath = "data/shaders/normal_debug/normal_debug.geom";
         mNormalEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDescNormal);
      };

      loadShaders();
   }

   void DebugJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.mainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      //renderTarget->AddDepthAttachment(gbuffer.depthImage, VK_ATTACHMENT_LOAD_OP_LOAD);
      mRenderTarget->SetClearColor(1, 1, 1, 1);
      mRenderTarget->Create();
   }

   void DebugJob::PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
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

         Model* model = renderable->GetModel();
         std::vector<RenderCommand> renderCommands;
         model->GetRenderCommands(renderCommands, renderable->GetTransform().GetWorldMatrix());

         if (renderable->HasRenderFlags(RENDER_FLAG_COLOR))
         {
            commandBuffer->CmdBindPipeline(mColorEffect->GetPipeline());
            commandBuffer->CmdBindDescriptorSets(mColorEffect);

            for (RenderCommand& command : renderCommands)
            {
               Vk::PushConstantBlock pushConsts(command.world, renderable->GetColor());
               commandBuffer->CmdPushConstants(mColorEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

               for (uint32_t i = 0; i < command.mesh->primitives.size(); i++)
               {
                  gRendererUtility().DrawPrimitive(commandBuffer, command.mesh->primitives[i]);
               }
            }
         }

         if (renderable->HasRenderFlags(RENDER_FLAG_NORMAL_DEBUG))
         {
            commandBuffer->CmdBindPipeline(mNormalEffect->GetPipeline());
            commandBuffer->CmdBindDescriptorSets(mNormalEffect);

            for (RenderCommand& command : renderCommands)
            {
               Vk::PushConstantBlock pushConsts(command.world, renderable->GetColor());
               commandBuffer->CmdPushConstants(mNormalEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

               for (uint32_t i = 0; i < command.mesh->primitives.size(); i++)
               {
                  gRendererUtility().DrawPrimitive(commandBuffer, command.mesh->primitives[i]);
               }
            }
         }
      }

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
