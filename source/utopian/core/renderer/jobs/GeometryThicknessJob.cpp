#include "core/renderer/jobs/GeometryThicknessJob.h"
#include "core/renderer/jobs/SSAOJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Model.h"

namespace Utopian
{
   GeometryThicknessJob::GeometryThicknessJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {

   }

   GeometryThicknessJob::~GeometryThicknessJob()
   {
   }

   void GeometryThicknessJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      geometryThicknessImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R32G32_SFLOAT, "Geometry thickness image");

      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddWriteOnlyColorAttachment(geometryThicknessImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->SetClearColor(DEFAULT_THICKNESS, 0.0f, 0.0f, 0.0f);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/geometry_thickness/geometry_thickness.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/geometry_thickness/geometry_thickness.frag";
      effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mEffect->BindCombinedImage("depthSampler", *gbuffer.depthImage, *mRenderTarget->GetSampler());

      // const uint32_t size = 640;
      // gScreenQuadUi().AddQuad(10, mHeight - (size + 10), size, size, geometryThicknessImage.get(), mRenderTarget->GetSampler());
   }

   void GeometryThicknessJob::Render(const JobInput& jobInput)
   {
      mRenderTarget->Begin("Geometry thickness pass", glm::vec4(0.5, 1.0, 1.0, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      if (IsEnabled())
      {
         // Todo: Should this be moved to the effect instead?
         commandBuffer->CmdBindPipeline(mEffect->GetPipeline());

         // Todo: Instanced objects

         /* Render all renderables */
         for (auto& renderable : jobInput.sceneInfo.renderables)
         {
            if (!renderable->IsVisible() || ((renderable->GetRenderFlags() & RENDER_FLAG_DEFERRED) != RENDER_FLAG_DEFERRED))
               continue;

            Model* model = renderable->GetModel();
            std::vector<RenderCommand> renderCommands;
            model->GetRenderCommands(renderCommands, renderable->GetTransform().GetWorldMatrix());

            for (RenderCommand& command : renderCommands)
            {
               Vk::PushConstantBlock pushConsts(command.world, renderable->GetColor());
               commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

               for (uint32_t i = 0; i < command.mesh->primitives.size(); i++)
               {
                  Primitive* primitive = command.mesh->primitives[i];

                  VkDescriptorSet materialDescriptorSet = command.mesh->materials[i].descriptorSet->GetVkHandle();
                  VkDescriptorSet descriptorSets[2] = { mEffect->GetDescriptorSet(0).GetVkHandle(), materialDescriptorSet };
                  commandBuffer->CmdBindDescriptorSet(mEffect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

                  gRendererUtility().DrawPrimitive(commandBuffer, primitive);
               }
            }
         }
      }

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
