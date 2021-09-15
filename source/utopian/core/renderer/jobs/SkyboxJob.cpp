#include "core/renderer/jobs/SkyboxJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Model.h"
#include "vulkan/TextureLoader.h"

namespace Utopian
{
   SkyboxJob::SkyboxJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   SkyboxJob::~SkyboxJob()
   {
   }

   void SkyboxJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage);
      mRenderTarget->SetClearColor(1, 1, 1, 1);
      mRenderTarget->Create();

      mSkybox = Vk::gTextureLoader().LoadCubemapTexture("data/textures/cubemap_space.ktx");

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/skybox/skybox.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/skybox/skybox.frag";
      effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
      effectDesc.pipelineDesc.depthStencilState.depthWriteEnable = VK_FALSE;
      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mInputBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mEffect->BindUniformBuffer("UBO_input", mInputBlock);
      mEffect->BindCombinedImage("samplerCubeMap", *mSkybox);

      mCubeModel = Vk::gModelLoader().LoadBox();
   }

   void SkyboxJob::Render(const JobInput& jobInput)
   {
      mInputBlock.data.world = glm::scale(glm::mat4(), glm::vec3(10000.0f));
      mInputBlock.UpdateMemory();

      mRenderTarget->Begin();
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(mEffect);

      Primitive* primitive = mCubeModel->GetPrimitive(0);
      commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());
      commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
      commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), 1, 0, 0, 0);

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
