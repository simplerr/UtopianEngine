#include "core/renderer/jobs/SkyboxJob.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Model.h"
#include "vulkan/TextureLoader.h"

namespace Utopian
{
   SkyboxJob::SkyboxJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      Vk::IMAGE_CREATE_INFO createInfo;
      createInfo.width = width;
      createInfo.height = height;
      createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
      createInfo.name = "Skybox sun image";
      createInfo.finalImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      createInfo.transitionToFinalLayout = true;

      sunImage = std::make_shared<Vk::Image>(createInfo, device);
   }

   SkyboxJob::~SkyboxJob()
   {
   }

   void SkyboxJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.mainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
      mRenderTarget->SetClearColor(1, 1, 1, 1);
      mRenderTarget->Create();

      Vk::TextureLoader& tl = Vk::gTextureLoader();
      mTexture = tl.LoadCubemapTexture("data/textures/environments/papermill.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);
      mIrradianceMap = tl.CreateCubemapTexture(VK_FORMAT_R32G32B32A32_SFLOAT, 64, 64, (uint32_t)floor(log2(64)) + 1);
      mSpecularMap = tl.CreateCubemapTexture(VK_FORMAT_R16G16B16A16_SFLOAT, 512, 512, (uint32_t)floor(log2(512)) + 1);

      gRendererUtility().FilterCubemap(mTexture.get(), mIrradianceMap.get(),
         "data/shaders/ibl_filtering/irradiance_filter.frag");

      gRendererUtility().FilterCubemap(mTexture.get(), mSpecularMap.get(),
         "data/shaders/ibl_filtering/specular_filter.frag");

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/skybox/skybox.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/skybox/skybox.frag";
      effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
      effectDesc.pipelineDesc.depthStencilState.depthWriteEnable = VK_FALSE;
      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mInputBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mEffect->BindUniformBuffer("UBO_input", mInputBlock);
      //mEffect->BindCombinedImage("samplerCubeMap", *mTexture);
      mEffect->BindCombinedImage("samplerCubeMap", *mIrradianceMap);

      mCubeModel = gModelLoader().LoadBox();
   }

   void SkyboxJob::Render(const JobInput& jobInput)
   {
      mInputBlock.data.world = glm::scale(glm::mat4(), glm::vec3(300.0f));
      mInputBlock.UpdateMemory();

      mRenderTarget->Begin("Skybox pass", glm::vec4(0.3, 0.8, 1.0, 1.0));
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
