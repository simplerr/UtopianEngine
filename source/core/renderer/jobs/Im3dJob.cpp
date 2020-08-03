#include "core/renderer/jobs/Im3dJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "im3d/im3d.h"
#include <im3d/im3d_config.h>
#include <vulkan/Effect.h>

namespace Utopian
{
    Im3dJob::Im3dJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   Im3dJob::~Im3dJob()
   {
   }

   void Im3dJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->SetClearColor(1, 1, 1, 1);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/im3d/im3d.vert";

      // Need to override vertex input description from shader since there is some special
      // treatment of U32 -> vec4 in Im3d
      mVertexDescription = std::make_shared<Vk::VertexDescription>();
      mVertexDescription->AddBinding(BINDING_0, sizeof(Im3d::VertexData), VK_VERTEX_INPUT_RATE_VERTEX);
      mVertexDescription->AddAttribute(BINDING_0, Vk::Vec4Attribute());
      mVertexDescription->AddAttribute(BINDING_0, Vk::U32Attribute());

      effectDesc.pipelineDesc.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
      effectDesc.pipelineDesc.depthStencilState.depthTestEnable = VK_FALSE;
      effectDesc.pipelineDesc.depthStencilState.depthWriteEnable = VK_FALSE;
      effectDesc.pipelineDesc.OverrideVertexInput(mVertexDescription);
      effectDesc.pipelineDesc.blendingType = Vk::BlendingType::BLENDING_ALPHA;

      Vk::EffectCreateInfo depthTestEffectDesc = effectDesc;
      depthTestEffectDesc.pipelineDesc.depthStencilState.depthTestEnable = VK_TRUE;
      depthTestEffectDesc.pipelineDesc.depthStencilState.depthWriteEnable = VK_TRUE;

      /* This is ugly and should be replaced by dynamic pipeline objects if possible,
         Or the Effect/Pipeline should be extended to support variations. */

      // Points
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/im3d/im3d_points.frag";
      mPointsEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      depthTestEffectDesc.shaderDesc.fragmentShaderPath = "data/shaders/im3d/im3d_points.frag";
      mPointsDepthTestEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), depthTestEffectDesc);

      // Triangles
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/im3d/im3d_triangles.frag";
      effectDesc.pipelineDesc.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      mTrianglesEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      depthTestEffectDesc.shaderDesc.fragmentShaderPath = "data/shaders/im3d/im3d_triangles.frag";
      depthTestEffectDesc.pipelineDesc.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      mTrianglesDepthTestEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), depthTestEffectDesc);

      // Lines
      effectDesc.shaderDesc.geometryShaderPath = "data/shaders/im3d/im3d.geom";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/im3d/im3d_lines.frag";
      effectDesc.pipelineDesc.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      mLinesEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      depthTestEffectDesc.shaderDesc.geometryShaderPath = "data/shaders/im3d/im3d.geom";
      depthTestEffectDesc.shaderDesc.fragmentShaderPath = "data/shaders/im3d/im3d_lines.frag";
      depthTestEffectDesc.pipelineDesc.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      mLinesDepthTestEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), depthTestEffectDesc);

      mLinesEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mPointsEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mTrianglesEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());

      mLinesDepthTestEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mPointsDepthTestEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mTrianglesDepthTestEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
   }

   void Im3dJob::Render(const JobInput& jobInput)
   {
      mRenderTarget->Begin("Im3d pass", glm::vec4(0.7, 0.2, 0.5, 1.0));

      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      commandBuffer->CmdBindVertexBuffer(0, 1, jobInput.sceneInfo.im3dVertices.get());

      uint32_t vertexOffset = 0;
      for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
      {
         const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

         if (drawList.m_layerId == IM3D_DEPTH_TESTING_LAYER)
            int a = 1;

         if (drawList.m_primType == Im3d::DrawPrimitive_Lines)
         {
            if (drawList.m_layerId == IM3D_DEPTH_TESTING_LAYER)
            {
               commandBuffer->CmdBindDescriptorSets(mLinesDepthTestEffect);
               commandBuffer->CmdBindPipeline(mLinesDepthTestEffect->GetPipeline());
            }
            else
            {
               commandBuffer->CmdBindDescriptorSets(mLinesEffect);
               commandBuffer->CmdBindPipeline(mLinesEffect->GetPipeline());
            }
         }
         else if (drawList.m_primType == Im3d::DrawPrimitive_Points)
         {
            if (drawList.m_layerId == IM3D_DEPTH_TESTING_LAYER)
            {
               commandBuffer->CmdBindDescriptorSets(mPointsDepthTestEffect);
               commandBuffer->CmdBindPipeline(mPointsDepthTestEffect->GetPipeline());
            }
            else
            {
               commandBuffer->CmdBindDescriptorSets(mPointsEffect);
               commandBuffer->CmdBindPipeline(mPointsEffect->GetPipeline());
            }
         }
         else if (drawList.m_primType == Im3d::DrawPrimitive_Triangles)
         {
            if (drawList.m_layerId == IM3D_DEPTH_TESTING_LAYER)
            {
               commandBuffer->CmdBindDescriptorSets(mTrianglesDepthTestEffect);
               commandBuffer->CmdBindPipeline(mTrianglesDepthTestEffect->GetPipeline());
            }
            else
            {
               commandBuffer->CmdBindDescriptorSets(mTrianglesEffect);
               commandBuffer->CmdBindPipeline(mTrianglesEffect->GetPipeline());
            }
         }

         commandBuffer->CmdDraw(drawList.m_vertexCount, 1, vertexOffset, 0);

         vertexOffset += drawList.m_vertexCount;
      }

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
