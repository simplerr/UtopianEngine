#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Model.h"
#include "core/Camera.h"
#include "vulkan/Debug.h"
#include "vulkan/handles/Queue.h"

namespace Utopian
{
   struct GBufferPushConstants
   {
      GBufferPushConstants(glm::mat4 _world, glm::vec4 _color = glm::vec4(1.0f), glm::vec2 tiling = glm::vec2(1.0f, 1.0f))
      {
         world = _world;
         color = _color;

         textureTiling = tiling;

         // Note: This needs to be done to have the physical world match the rendered world.
         // See https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/ for more information.
         world[3][0] = -world[3][0];
         world[3][1] = -world[3][1];
         world[3][2] = -world[3][2];
      }
   
      glm::mat4 world;
      glm::vec4 color; // r, g, b, brightness
      glm::vec2 textureTiling;
      glm::vec2 pad;
   };

   GBufferJob::GBufferJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {

   }

   GBufferJob::~GBufferJob()
   {
   }

   void GBufferJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.positionImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.normalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.albedoImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.normalViewImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRenderTarget->AddReadWriteColorAttachment(gbuffer.specularImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage);
      mRenderTarget->SetClearColor(0, 0, 0, 1);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/gbuffer/gbuffer.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/gbuffer/gbuffer.frag";
      mGBufferEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      Vk::EffectCreateInfo effectDescLine;
      effectDescLine.shaderDesc.vertexShaderPath = "data/shaders/gbuffer/gbuffer.vert";
      effectDescLine.shaderDesc.fragmentShaderPath = "data/shaders/gbuffer/gbuffer.frag";
      effectDescLine.pipelineDesc.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
      mGBufferEffectWireframe = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDescLine);

      // Override vertex description for instancing shaders
      SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>(Vk::Vertex::GetDescription());
      vertexDescription->AddBinding(BINDING_1, sizeof(InstanceDataGPU), VK_VERTEX_INPUT_RATE_INSTANCE);
      vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());  // Location 0 : InInstanceWorld
      vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());  // Location 1 : InInstanceWorld
      vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());  // Location 2 : InInstanceWorld
      vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());  // Location 3 : InInstanceWorld

      Vk::EffectCreateInfo effectDescInstancingAnimation;
      effectDescInstancingAnimation.shaderDesc.vertexShaderPath = "data/shaders/gbuffer/gbuffer_instancing_animation.vert";
      effectDescInstancingAnimation.shaderDesc.fragmentShaderPath = "data/shaders/gbuffer/gbuffer.frag";
      effectDescInstancingAnimation.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
      effectDescInstancingAnimation.pipelineDesc.OverrideVertexInput(vertexDescription);
      mInstancedAnimationEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDescInstancingAnimation);

      Vk::EffectCreateInfo effectDescInstancing;
      effectDescInstancing.shaderDesc.vertexShaderPath = "data/shaders/gbuffer/gbuffer_instancing.vert";
      effectDescInstancing.shaderDesc.fragmentShaderPath = "data/shaders/gbuffer/gbuffer.frag";
      effectDescInstancing.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
      effectDescInstancing.pipelineDesc.OverrideVertexInput(vertexDescription);
      mGBufferEffectInstanced = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDescInstancing);

      mInstancedAnimationEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mGBufferEffectInstanced->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mGBufferEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mGBufferEffectWireframe->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());

      mSettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mGBufferEffect->BindUniformBuffer("UBO_settings", mSettingsBlock);
      mGBufferEffectWireframe->BindUniformBuffer("UBO_settings", mSettingsBlock);
      mInstancedAnimationEffect->BindUniformBuffer("UBO_settings", mSettingsBlock);
      mGBufferEffectInstanced->BindUniformBuffer("UBO_settings", mSettingsBlock);

      mFoliageSpheresBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mInstancedAnimationEffect->BindUniformBuffer("UBO_sphereList", mFoliageSpheresBlock);

      mAnimationParametersBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mInstancedAnimationEffect->BindUniformBuffer("UBO_animationParameters", mAnimationParametersBlock);

      mWindmapTexture = Vk::gTextureLoader().LoadTexture("data/textures/windmap.jpg");
      mInstancedAnimationEffect->BindCombinedImage("windmapSampler", *mWindmapTexture);
   }

   void GBufferJob::Render(const JobInput& jobInput)
   {
      mSettingsBlock.data.normalMapping = jobInput.renderingSettings.normalMapping;
      mSettingsBlock.UpdateMemory();

      // Note: use hardcoded size if no Terrain is available
      if (jobInput.sceneInfo.terrain != nullptr)
         mAnimationParametersBlock.data.terrainSize = jobInput.sceneInfo.terrain->GetTerrainSize();
      else
         mAnimationParametersBlock.data.terrainSize = 128.0 * 511;

      mAnimationParametersBlock.data.strength = jobInput.renderingSettings.windStrength;
      mAnimationParametersBlock.data.frequency = jobInput.renderingSettings.windFrequency;
      mAnimationParametersBlock.data.enabled = jobInput.renderingSettings.windEnabled;
      mAnimationParametersBlock.UpdateMemory();

      // Collect renderables that should affect the vegetation
      uint32_t nextSphereIndex = 0;
      for (auto& renderable : jobInput.sceneInfo.renderables)
      {
         if (renderable->IsPushingFoliage())
         {
            mFoliageSpheresBlock.spheres[nextSphereIndex].position = renderable->GetPosition();
            mFoliageSpheresBlock.spheres[nextSphereIndex].radius = renderable->GetBoundingBox().GetRadius();
            mFoliageSpheresBlock.constants.padding = glm::vec3(13.37f);
            nextSphereIndex++;
         }
      }

      mFoliageSpheresBlock.constants.numSpheres = (float)nextSphereIndex;
      if (mFoliageSpheresBlock.constants.numSpheres > 0)
         mFoliageSpheresBlock.UpdateMemory();

      mRenderTarget->Begin("G-buffer pass", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      /* Render instanced assets */
      for (uint32_t i = 0; i < jobInput.sceneInfo.instanceGroups.size(); i++)
      {
         SharedPtr<InstanceGroup> instanceGroup = jobInput.sceneInfo.instanceGroups[i];
         Vk::Buffer* instanceBuffer = instanceGroup->GetBuffer();
         Model* model = instanceGroup->GetModel();

         if (instanceBuffer != nullptr && model != nullptr)
         {
            SharedPtr<Vk::Effect> effect = nullptr;
            if (!instanceGroup->IsAnimated())
               effect = mGBufferEffectInstanced;
            else
               effect = mInstancedAnimationEffect;

            commandBuffer->CmdBindPipeline(effect->GetPipeline());

            float modelHeight = model->GetBoundingBox().GetHeight();

            // Todo: Perhaps they can share the same shader and just have a flag for doing animation
            if (instanceGroup->IsAnimated())
            {
               // Push the world matrix constant
               InstancePushConstantBlock pushConsts(modelHeight);
               commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);
            }

            std::vector<RenderCommand> renderCommands;
            model->GetRenderCommands(renderCommands, glm::mat4());
            
            for (RenderCommand& command : renderCommands)
            {
               for (uint32_t i = 0; i < command.mesh->primitives.size(); i++)
               {
                  Primitive* primitive = command.mesh->primitives[i];

                  VkDescriptorSet materialDescriptorSet = command.mesh->materials[i].descriptorSet->GetVkHandle();
                  VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).GetVkHandle(), materialDescriptorSet };
                  commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);
                  commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());
                  commandBuffer->CmdBindVertexBuffer(1, 1, instanceBuffer);
                  commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                  commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), instanceGroup->GetNumInstances(), 0, 0, 0);
               }
            }
         }
      }

      /* Render all renderables */
      for (auto& renderable : jobInput.sceneInfo.renderables)
      {
         if (!renderable->IsVisible() || !(renderable->HasRenderFlags(RENDER_FLAG_DEFERRED) || renderable->HasRenderFlags(RENDER_FLAG_WIREFRAME)))
            continue;

         Vk::Effect* effect = mGBufferEffect.get();
         if (renderable->HasRenderFlags(RENDER_FLAG_WIREFRAME))
            effect = mGBufferEffectWireframe.get();

         commandBuffer->CmdBindPipeline(effect->GetPipeline());

         Model* model = renderable->GetModel();
         std::vector<RenderCommand> renderCommands;
         model->GetRenderCommands(renderCommands, renderable->GetTransform().GetWorldMatrix());

         for (RenderCommand& command : renderCommands)
         {
            GBufferPushConstants pushConsts(command.world, renderable->GetColor(), renderable->GetTextureTiling());
            commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

            for (uint32_t i = 0; i < command.mesh->primitives.size(); i++)
            {
               Primitive* primitive = command.mesh->primitives[i];

               VkDescriptorSet materialDescriptorSet = command.mesh->materials[i].descriptorSet->GetVkHandle();
               VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).GetVkHandle(), materialDescriptorSet };
               commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

               gRendererUtility().DrawPrimitive(commandBuffer, primitive);
            }
         }
      }

      Vk::DebugLabel::EndRegion(commandBuffer->GetVkHandle());

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}