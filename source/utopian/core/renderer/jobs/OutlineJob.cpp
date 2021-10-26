#include "core/renderer/jobs/OutlineJob.h"
#include "core/renderer/jobs/TonemapJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include <core/renderer/Renderable.h>
#include <core/renderer/Model.h>
#include "utility/math/Helpers.h"
#include <random>

namespace Utopian
{
   OutlineJob::OutlineJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   OutlineJob::~OutlineJob()
   {
   }

   void OutlineJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      InitMaskPass(jobs, gbuffer);
      InitEdgePass(jobs, gbuffer);
   }

   void OutlineJob::Render(const JobInput& jobInput)
   {
      RenderMaskPass(jobInput);
      RenderEdgePass(jobInput);
   }

   void OutlineJob::InitMaskPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mMaskPass.image = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "Outline image");

      mMaskPass.renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mMaskPass.renderTarget->AddWriteOnlyColorAttachment(mMaskPass.image);
      mMaskPass.renderTarget->SetClearColor(0, 0, 0, 0);
      mMaskPass.renderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/color/color.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/color/color.frag";
      mMaskPass.effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mMaskPass.renderTarget->GetRenderPass(), effectDesc);

      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/color/color_skinning.vert";
      mMaskPass.effectSkinning = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mMaskPass.renderTarget->GetRenderPass(), effectDesc);

      mMaskPass.effect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mMaskPass.effectSkinning->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
   }

   void OutlineJob::InitEdgePass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      TonemapJob* tonemapJob = static_cast<TonemapJob*>(jobs[JobGraph::TONEMAP_INDEX]);

      mEdgePass.image = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "Edge image");

      mEdgePass.renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mEdgePass.renderTarget->AddReadWriteColorAttachment(tonemapJob->outputImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mEdgePass.renderTarget->SetClearColor(0, 0, 0, 1);
      mEdgePass.renderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/outline/outline.frag";
      effectDesc.pipelineDesc.blendingType = Vk::BLENDING_ALPHA;
      mEdgePass.effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mEdgePass.renderTarget->GetRenderPass(), effectDesc);

      mEdgePass.settingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mEdgePass.effect->BindUniformBuffer("UBO_settings", mEdgePass.settingsBlock);
      mEdgePass.effect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mEdgePass.effect->BindCombinedImage("maskSampler", *mMaskPass.image, *mMaskPass.renderTarget->GetSampler());
   }

   void OutlineJob::RenderMaskPass(const JobInput& jobInput)
   {
      mMaskPass.renderTarget->Begin("Mask pass", glm::vec4(0.9, 1.0, 0.1, 1.0));
      Vk::CommandBuffer* commandBuffer = mMaskPass.renderTarget->GetCommandBuffer();

      if (IsEnabled())
      {
         for (auto& renderable : jobInput.sceneInfo.renderables)
         {
            if (renderable->IsVisible() && renderable->HasRenderFlags(RENDER_FLAG_DRAW_OUTLINE))
            {
               Model* model = renderable->GetModel();
               std::vector<RenderCommand> renderCommands;
               model->GetRenderCommands(renderCommands, renderable->GetTransform().GetWorldMatrix());

               Vk::Effect* effect = mMaskPass.effect.get();
               if (model->IsAnimated())
                  effect = mMaskPass.effectSkinning.get();

               commandBuffer->CmdBindPipeline(effect->GetPipeline());

               VkDescriptorSet descriptorSets[1] = { effect->GetDescriptorSet(0).GetVkHandle() };
               commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 1, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);

               for (RenderCommand& command : renderCommands)
               {
                  if (command.skinDescriptorSet != VK_NULL_HANDLE)
                  {
                     commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 1, &command.skinDescriptorSet,
                                                         VK_PIPELINE_BIND_POINT_GRAPHICS, 1);
                  }

                  Vk::PushConstantBlock pushConsts(command.world, renderable->GetColor());
                  commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

                  for (uint32_t i = 0; i < command.mesh->primitives.size(); i++)
                  {
                     Primitive* primitive = command.mesh->primitives[i];
                     gRendererUtility().DrawPrimitive(commandBuffer, primitive);
                  }
               }
            }
         }
      }

      mMaskPass.renderTarget->End(GetWaitSemahore(), mMaskPass.semaphore);
   }

   void OutlineJob::RenderEdgePass(const JobInput& jobInput)
   {
      mEdgePass.settingsBlock.data.outlineWidth = jobInput.renderingSettings.outlineWidth;
      mEdgePass.settingsBlock.UpdateMemory();

      mEdgePass.renderTarget->Begin("Edge pass", glm::vec4(0.5, 0.1, 0.3, 1.0));

      if (IsEnabled())
      {
         Vk::CommandBuffer* commandBuffer = mEdgePass.renderTarget->GetCommandBuffer();
         commandBuffer->CmdBindPipeline(mEdgePass.effect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mEdgePass.effect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mEdgePass.renderTarget->End(mMaskPass.semaphore, GetCompletedSemahore());
   }
}
