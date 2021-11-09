#include "core/renderer/jobs/AtmosphereJob.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Light.h"
#include "core/renderer/Primitive.h"
#include "core/renderer/Model.h"
#include "core/Camera.h"
#include "core/Input.h"
#include <vulkan/vulkan_core.h>

namespace Utopian
{
   AtmosphereJob::AtmosphereJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      sunImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R8G8B8A8_UNORM, "Atmosphere sun image");
      mEnvironmentGenerated = false;
   }

   AtmosphereJob::~AtmosphereJob()
   {
   }

   void AtmosphereJob::LoadResources()
   {
      auto loadShader = [&]()
      {
         Vk::EffectCreateInfo effectDesc;
         effectDesc.shaderDesc.vertexShaderPath = "data/shaders/atmosphere/atmosphere.vert";
         effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/atmosphere/atmosphere.frag";
         effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
         effectDesc.pipelineDesc.depthStencilState.depthWriteEnable = VK_FALSE;
         mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);
      };

      loadShader();
   }

   void AtmosphereJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.mainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mRenderTarget->AddWriteOnlyColorAttachment(sunImage);
      mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
      mRenderTarget->SetClearColor(0, 0, 0);
      mRenderTarget->Create();

      mParameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      mSkydomeModel = gModelLoader().LoadModel("data/models/sphere.obj");

      //environmentCube = Vk::gTextureLoader().LoadCubemapTexture("data/textures/environments/papermill.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);
      environmentCube = Vk::gTextureLoader().CreateCubemapTexture(VK_FORMAT_R32G32B32A32_SFLOAT, 512, 512, (uint32_t)floor(log2(512)) + 1);
      irradianceMap = Vk::gTextureLoader().CreateCubemapTexture(VK_FORMAT_R32G32B32A32_SFLOAT, 64, 64, (uint32_t)floor(log2(64)) + 1);
      specularMap = Vk::gTextureLoader().CreateCubemapTexture(VK_FORMAT_R16G16B16A16_SFLOAT, 512, 512, (uint32_t)floor(log2(512)) + 1);

      // const uint32_t size = 240;
      // gScreenQuadUi().AddQuad(5 * (size + 10) + 10, mHeight - (2 * size + 10), size, size, sunImage.get(), mRenderTarget->GetSampler());
   }

   void AtmosphereJob::PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mEffect->BindUniformBuffer("UBO_atmosphere", mParameterBlock);
   }

   void AtmosphereJob::CaptureEnvironmentCubemap(glm::vec3 sunDir)
   {
      mParameterBlock.data.sunDir = sunDir;
      mParameterBlock.UpdateMemory();

      const uint32_t dimension = environmentCube->GetWidth();
      const VkFormat format = environmentCube->GetImage().GetFormat();
      const uint32_t numMipLevels = (uint32_t)floor(log2(dimension)) + 1;
      //environmentCube = Vk::gTextureLoader().CreateCubemapTexture(format, dimension, dimension, numMipLevels);

      // Offscreen framebuffer
      SharedPtr<Vk::Image> offscreen = std::make_shared<Vk::ImageColor>(mDevice, dimension,
         dimension, format, "Offscreen atmosphere cubemap capture image");
      SharedPtr<Vk::ImageDepth> depthImage = std::make_shared<Vk::ImageDepth>(mDevice, mWidth, mHeight, VK_FORMAT_D32_SFLOAT_S8_UINT, "G-buffer depth image");

      SharedPtr<Vk::RenderTarget> renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, dimension, dimension);
      renderTarget->AddWriteOnlyColorAttachment(offscreen);
      renderTarget->AddWriteOnlyColorAttachment(sunImage);
      renderTarget->AddWriteOnlyDepthAttachment(depthImage);
      renderTarget->SetClearColor(1, 1, 1, 1);
      renderTarget->Create();

      glm::mat4 matrices[] =
      {
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
      };

      Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();
      commandBuffer->Begin();
      renderTarget->BeginDebugLabelAndQueries("Irradiance cubemap generation", glm::vec4(1.0f));

      environmentCube->GetImage().LayoutTransition(*commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

      for (uint32_t mipLevel = 0; mipLevel < numMipLevels; mipLevel++)
      {
         for (uint32_t face = 0; face < 6; face++)
         {
            renderTarget->BeginRenderPass();

            float viewportSize = dimension * (float)std::pow(0.5f, mipLevel);
            commandBuffer->CmdSetViewPort(viewportSize, viewportSize);

            struct PushConsts {
               glm::mat4 projection;
               glm::mat4 view;
               glm::mat4 world;
            } pushConsts;

            pushConsts.view = matrices[face];
            pushConsts.projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 20000.0f);
            glm::mat4 world = glm::rotate(glm::mat4(), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            pushConsts.world = glm::scale(world, glm::vec3(16.0f));

            commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(PushConsts), &pushConsts.projection);

            commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
            commandBuffer->CmdBindDescriptorSets(mEffect);

            Primitive* primitive = mSkydomeModel->GetPrimitive(0);
            commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());
            commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), 1, 0, 0, 0);

            commandBuffer->CmdEndRenderPass();

            // Copy region for transfer from framebuffer to cube face
            VkImageCopy copyRegion = {};

            copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.mipLevel = 0;
            copyRegion.srcSubresource.layerCount = 1;
            copyRegion.srcOffset = { 0, 0, 0 };

            copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.dstSubresource.baseArrayLayer = face;
            copyRegion.dstSubresource.mipLevel = mipLevel;
            copyRegion.dstSubresource.layerCount = 1;
            copyRegion.dstOffset = { 0, 0, 0 };

            copyRegion.extent.width = static_cast<uint32_t>(viewportSize);
            copyRegion.extent.height = static_cast<uint32_t>(viewportSize);
            copyRegion.extent.depth = 1;

            offscreen->LayoutTransition(*commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            vkCmdCopyImage(commandBuffer->GetVkHandle(), offscreen->GetVkHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, environmentCube->GetImage().GetVkHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            offscreen->LayoutTransition(*commandBuffer, offscreen->GetFinalLayout());
         }
      }

      environmentCube->GetImage().LayoutTransition(*commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      renderTarget->EndDebugLabelAndQueries();
      commandBuffer->Flush();
   }

   void AtmosphereJob::PreRender(const JobInput& jobInput)
   {
      // If there is a new directional light in the scene then the
      // environment map needs to be recaptured. Note: if the environment
      // capturing is moved to somewhere else this could be cleaned up.
      static Light* directionalLight = jobInput.sceneInfo.directionalLight;
      bool newLight = false;
      if (directionalLight != jobInput.sceneInfo.directionalLight)
      {
         newLight = true;
         directionalLight = jobInput.sceneInfo.directionalLight;
      }

      if (newLight || !mEnvironmentGenerated || gInput().KeyPressed('R'))
      {
         glm::vec3 sunDir = glm::vec3(0.0f);
         if (jobInput.sceneInfo.directionalLight != nullptr)
            sunDir = jobInput.sceneInfo.directionalLight->GetDirection();

         CaptureEnvironmentCubemap(sunDir);

         gRendererUtility().FilterCubemap(environmentCube.get(), irradianceMap.get(),
            "data/shaders/ibl_filtering/irradiance_filter.frag");

         gRendererUtility().FilterCubemap(environmentCube.get(), specularMap.get(),
            "data/shaders/ibl_filtering/specular_filter.frag");

         mEnvironmentGenerated = true;
      }
   }

   void AtmosphereJob::Render(const JobInput& jobInput)
   {
      if (jobInput.sceneInfo.directionalLight != nullptr)
         mParameterBlock.data.sunDir = jobInput.sceneInfo.directionalLight->GetDirection();
      else
         mParameterBlock.data.sunDir = glm::vec3(0.0f);

      mParameterBlock.UpdateMemory();

      mRenderTarget->Begin("Atmosphere pass", glm::vec4(0.1, 0.8, 0.8, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      struct PushConsts {
         glm::mat4 projection;
         glm::mat4 view;
         glm::mat4 world;
      } pushConsts;

      pushConsts.view = gRenderer().GetMainCamera()->GetView();
      pushConsts.projection = gRenderer().GetMainCamera()->GetProjection();
      glm::mat4 world = glm::rotate(glm::mat4(), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      pushConsts.world = glm::scale(world, glm::vec3(16.0f));

      commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(PushConsts), &pushConsts.projection);

      // Todo: Should this be moved to the effect instead?
      commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(mEffect);

      Primitive* primitive = mSkydomeModel->GetPrimitive(0);
      commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());
      commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
      commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), 1, 0, 0, 0);

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
