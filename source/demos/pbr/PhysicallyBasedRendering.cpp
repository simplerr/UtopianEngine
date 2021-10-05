#include "PhysicallyBasedRendering.h"
#include "core/renderer/Model.h"
#include "core/glTFLoader.h"
#include <glm/matrix.hpp>
#include <imgui/imgui.h>
#include <string>
#include <time.h>
#include <vulkan/handles/DescriptorSet.h>
#include <vulkan/vulkan_core.h>
#include "core/Input.h"
#include "core/Log.h"
#include "core/renderer/Renderer.h"
#include "core/Window.h"
#include "core/LuaManager.h"
#include "core/Profiler.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/RendererUtility.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "vulkan/Debug.h"
#include "vulkan/handles/Device.h"
#include "vulkan/EffectManager.h"
#include "vulkan/TextureLoader.h"
#include "core/ModelLoader.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/CommandBuffer.h"
#include "core/MiniCamera.h"
#include "core/Engine.h"
#include "core/AssimpLoader.h"
#include "utility/Timer.h"
#include "core/renderer/Primitive.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "nativefiledialog/nfd.h"

PhysicallyBasedRendering::PhysicallyBasedRendering(Utopian::Window* window)
   : mWindow(window)
{
   // Start Utopian Engine
   Utopian::gEngine().Start(window, "PBR Demo");
   Utopian::gEngine().StartModules();
   Utopian::gEngine().RegisterUpdateCallback(&PhysicallyBasedRendering::UpdateCallback, this);
   Utopian::gEngine().RegisterRenderCallback(&PhysicallyBasedRendering::DrawCallback, this);
   Utopian::gEngine().RegisterDestroyCallback(&PhysicallyBasedRendering::DestroyCallback, this);

   mVulkanApp = Utopian::gEngine().GetVulkanApp();

   // Wait on the terrain job before submitting the frame
   mPhysicallyBasedRenderingComplete = std::make_shared<Vk::Semaphore>(mVulkanApp->GetDevice());
   mVulkanApp->SetWaitSubmitSemaphore(mPhysicallyBasedRenderingComplete);

   InitResources();

   gModelLoader().SetInverseTranslation(false);

   //AddModel("data/models/gltf/Sponza/glTF/Sponza.gltf", glm::vec3(0.0f), glm::vec3(1.0f));
   // AddModel("data/models/gltf/CesiumMan.gltf", glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(1.0f),
   //          glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
   Utopian::Model* model = AddModel("data/models/gltf/FlightHelmet/glTF/FlightHelmet.gltf", glm::vec3(-1.5f, 1.0f, 0.0f), glm::vec3(1.0f));
   mModelInspector = std::make_shared<ModelInspector>(model);
   //mSelectedModel = AddModel("data/models/gltf/DamagedHelmet/glTF/DamagedHelmet.gltf", glm::vec3(-1.5f, 1.0f, 0.0f), glm::vec3(1.0f));
   
   // SceneNode sceneNode;
   // sceneNode.model = gModelLoader().LoadBox();
   // sceneNode.worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f)) *
   //                         glm::mat4(glm::quat()) *
   //                         glm::scale(glm::mat4(1.0f), glm::vec3(1.0));
   // mSceneNodes.push_back(sceneNode);

   //AddModel("data/models/sphere.obj", glm::vec3(1.0f, 0.0f, 3), glm::vec3(0.02f));
   const uint32_t max = 5;
   for (uint32_t x = 0; x < max; x++)
   {
      for (uint32_t y = 0; y < max; y++)
      {
         Utopian::Model* model = AddModel("data/models/gltf/sphere.gltf", glm::vec3(x, y, 0.0f), glm::vec3(0.4f));
         Utopian::Material* material = model->GetMaterial(0);
         material->properties->data.baseColorFactor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
         material->properties->data.metallicFactor = (float)y / (float)max;
         material->properties->data.roughnessFactor = 1.0f - ((float)x / (float)max);
         material->properties->data.occlusionFactor = 0.5;
         material->properties->UpdateMemory();
      }
   }

   AddModel("data/models/gltf/Fox/glTF/Fox.gltf", glm::vec3(2.0f, 0.25f, 1.78f), glm::vec3(0.01f));
}

PhysicallyBasedRendering::~PhysicallyBasedRendering()
{
   Utopian::gEngine().Destroy();
}

void PhysicallyBasedRendering::DestroyCallback()
{
   // Todo: Remove this when the engine shutdown sequence is improved
   mRenderTarget = nullptr;
   mEffect = nullptr;
   mSkinningEffect = nullptr;
   mPhysicallyBasedRenderingComplete = nullptr;
   mOutputImage = nullptr;
   mDepthImage = nullptr;
   mSampler = nullptr;
   mSkybox.effect = nullptr;
   mSkybox.model = nullptr;
   mSkybox.texture = nullptr;
   mIrradianceMap = nullptr;
   mSpecularMap = nullptr;
   mBRDFLut = nullptr;

   for (auto& sceneNode : mSceneNodes)
      sceneNode.model = nullptr;

   mVertexInputParameters.GetBuffer()->Destroy();
   mPbrSettings.GetBuffer()->Destroy();
   mSkybox.inputBlock.GetBuffer()->Destroy();
   mSkybox.shaderVariables.GetBuffer()->Destroy();
}

void PhysicallyBasedRendering::InitResources()
{
   uint32_t width = mWindow->GetWidth();
   uint32_t height = mWindow->GetHeight();
   Vk::Device* device = mVulkanApp->GetDevice();

   mCamera = std::make_shared<MiniCamera>(glm::vec3(1.31, 1.0, 5.58), glm::vec3(0, 0, 0), 0.01, 20000, 0.009f, width, height);

   mOutputImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R32G32B32A32_SFLOAT, "PhysicallyBasedRendering image");
   mDepthImage = std::make_shared<Vk::ImageDepth>(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT, "Depth image");
   mSampler = std::make_shared<Vk::Sampler>(device);
   mVertexInputParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
   mPbrSettings.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
   mPbrSettings.data.debugChannel = 0;
   mPbrSettings.data.useIBL = 1;

   mRenderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
   mRenderTarget->AddWriteOnlyColorAttachment(mOutputImage);
   mRenderTarget->AddWriteOnlyDepthAttachment(mDepthImage);
   mRenderTarget->SetClearColor(1, 1, 1, 1);
   mRenderTarget->Create();

   // Note: Todo: Hardcoded path to C:/Git/UtopianEngine/.
   Vk::EffectCreateInfo effectDesc;
   effectDesc.shaderDesc.vertexShaderPath = "C:/Git/UtopianEngine/source/demos/pbr/shaders/pbr.vert";
   effectDesc.shaderDesc.fragmentShaderPath = "C:/Git/UtopianEngine/source/demos/pbr/shaders/pbr.frag";
   effectDesc.pipelineDesc.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
   mEffect = Vk::Effect::Create(device, mRenderTarget->GetRenderPass(), effectDesc);

   effectDesc.shaderDesc.vertexShaderPath = "C:/Git/UtopianEngine/source/demos/pbr/shaders/pbr_skinning.vert";
   mSkinningEffect = Vk::Effect::Create(device, mRenderTarget->GetRenderPass(), effectDesc);

   mEffect->BindUniformBuffer("UBO_input", mVertexInputParameters);
   mEffect->BindUniformBuffer("UBO_settings", mPbrSettings);
   mSkinningEffect->BindUniformBuffer("UBO_input", mVertexInputParameters);
   mSkinningEffect->BindUniformBuffer("UBO_settings", mPbrSettings);

   InitSkybox();
   GenerateFilteredCubemaps();

   mBRDFLut = Vk::gTextureLoader().LoadTexture("data/textures/brdf_lut.ktx", VK_FORMAT_R16G16_SFLOAT);
   mBRDFLut->GetSampler().createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   mBRDFLut->GetSampler().createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   mBRDFLut->GetSampler().createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   mBRDFLut->GetSampler().Create();
   mBRDFLut->UpdateDescriptor();

   mEffect->BindCombinedImage("irradianceMap", *mIrradianceMap);
   mEffect->BindCombinedImage("specularMap", *mSpecularMap);
   mEffect->BindCombinedImage("brdfLut", *mBRDFLut);

   mSkinningEffect->BindCombinedImage("irradianceMap", *mIrradianceMap);
   mSkinningEffect->BindCombinedImage("specularMap", *mSpecularMap);
   mSkinningEffect->BindCombinedImage("brdfLut", *mBRDFLut);

   mSkybox.effect->BindCombinedImage("samplerCubeMap", *mIrradianceMap);

   gScreenQuadUi().AddQuad(0, 0, width, height, mOutputImage.get(), mSampler.get());
}

void PhysicallyBasedRendering::UpdateCubemapBindings()
{
   Vk::DescriptorSet& descriptorSet = mSkybox.effect->GetDescriptorSetFromName("samplerCubeMap");
   descriptorSet.BindCombinedImage("samplerCubeMap", mIrradianceMap->GetDescriptor());
   mVulkanApp->GetDevice()->QueueDescriptorUpdate(&descriptorSet);

   Vk::DescriptorSet& descriptorSet1 = mEffect->GetDescriptorSetFromName("irradianceMap");
   descriptorSet1.BindCombinedImage("irradianceMap", mIrradianceMap->GetDescriptor());
   descriptorSet1.BindCombinedImage("specularMap", mSpecularMap->GetDescriptor());
   mVulkanApp->GetDevice()->QueueDescriptorUpdate(&descriptorSet1);

   Vk::DescriptorSet& descriptorSet2 = mSkinningEffect->GetDescriptorSetFromName("irradianceMap");
   descriptorSet2.BindCombinedImage("irradianceMap", mIrradianceMap->GetDescriptor());
   descriptorSet2.BindCombinedImage("specularMap", mSpecularMap->GetDescriptor());
   mVulkanApp->GetDevice()->QueueDescriptorUpdate(&descriptorSet2);
}

void PhysicallyBasedRendering::UpdateCallback()
{
   glm::vec3 cameraPos = mCamera->GetPosition();

   ImGuiRenderer::BeginWindow("PBR Demo", glm::vec2(10, 150), 300.0f);
   ImGui::Text("Camera pos: (%.2f %.2f %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);

   static int selectedEnvironment = 0u;
   if (ImGui::Combo("Environment", &selectedEnvironment, "Papermill\0Uffizi\0...\0"))
   {
      std::string environment;
      bool status = true;
      if (selectedEnvironment == 0) {
         environment = "data/textures/environments/papermill.ktx";
      }
      else if (selectedEnvironment == 1) {
         environment = "data/textures/environments/uffizi_cube.ktx";
      }
      else if (selectedEnvironment == 2) {
         nfdchar_t* outPath = NULL;
         nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);
         if (result == NFD_OKAY)
            environment = outPath;
         else
            status = false;
      }

      if (status) {
         mSkybox.texture = Vk::gTextureLoader().LoadCubemapTexture(environment, VK_FORMAT_R16G16B16A16_SFLOAT);

         GenerateFilteredCubemaps();
         UpdateCubemapBindings();
      }
      else
         UTO_LOG("Error selecting environment map");
   }

   static int selectedCubemap = 0u;
   if (ImGui::Combo("Skybox style", &selectedCubemap, "Irradiance\0Specular\0Environment\0"))
   {
      Vk::DescriptorSet& descriptorSet = mSkybox.effect->GetDescriptorSetFromName("samplerCubeMap");

      if (selectedCubemap == 0) {
         descriptorSet.BindCombinedImage("samplerCubeMap", mIrradianceMap->GetDescriptor());
         mVulkanApp->GetDevice()->QueueDescriptorUpdate(&descriptorSet);
      }
      else if (selectedCubemap == 1) {
         descriptorSet.BindCombinedImage("samplerCubeMap", mSpecularMap->GetDescriptor());
         mVulkanApp->GetDevice()->QueueDescriptorUpdate(&descriptorSet);
      }
      else {
         descriptorSet.BindCombinedImage("samplerCubeMap", mSkybox.texture->GetDescriptor());
         mVulkanApp->GetDevice()->QueueDescriptorUpdate(&descriptorSet);
      }
   }

   ImGui::Combo("Debug channel", &mPbrSettings.data.debugChannel,
                "None\0Base color\0Metallic\0Roughness\0Normal\0Tangent\0Ambient Occlusion\0Irradiance\0Ambient\0Specular\0");

   static bool useIBL = mPbrSettings.data.useIBL;
   ImGui::Checkbox("IBL", &useIBL);
   mPbrSettings.data.useIBL = useIBL;

   mModelInspector->UpdateUi();

   ImGuiRenderer::EndWindow();

   static double prevTime = Utopian::gTimer().GetTime();
   double currentTime = Utopian::gTimer().GetTime();
   
   double deltaTime = currentTime - prevTime;
   prevTime = currentTime;

   for (auto& sceneNode : mSceneNodes)
      sceneNode.model->UpdateAnimation((float)deltaTime / 1000.0f);

   // Recompile shaders
   if (gInput().KeyPressed('R'))
   {
      Vk::gEffectManager().RecompileModifiedShaders();
   }

   mCamera->Update();
}

void PhysicallyBasedRendering::DrawCallback()
{
   // Update uniforms
   mVertexInputParameters.data.eyePos = glm::vec4(mCamera->GetPosition(), 1.0f);
   mVertexInputParameters.data.view = mCamera->GetView();
   mVertexInputParameters.data.projection = mCamera->GetProjection();
   mVertexInputParameters.UpdateMemory();

   mPbrSettings.UpdateMemory();

   // Material* material = mSceneNodes[0].model->GetMaterial(0);
   // material->properties->UpdateMemory();

   mRenderTarget->Begin("PBR pass", glm::vec4(0.3, 0.6, 0.9, 1.0));
   Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

   for (auto& sceneNode : mSceneNodes)
   {
      SharedPtr<Vk::Effect> effect = nullptr;
      if (sceneNode.model->IsAnimated())
         effect = mSkinningEffect;
      else
         effect = mEffect;

      commandBuffer->CmdBindPipeline(effect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(effect);

      // Test new method for drawing
      std::vector<RenderCommand> renderCommands;
      sceneNode.model->GetRenderCommands(renderCommands, sceneNode.worldMatrix);

      for (RenderCommand& command : renderCommands)
      {
         if (command.skinDescriptorSet != VK_NULL_HANDLE)
         {
            commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 1, &command.skinDescriptorSet,
                                                VK_PIPELINE_BIND_POINT_GRAPHICS, 4);
         }

         commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL,
                                         sizeof(glm::mat4), &command.world);

         for (uint32_t i = 0; i < command.mesh->primitives.size(); i++)
         {
            Primitive* primitive = command.mesh->primitives[i];

            VkDescriptorSet descriptorSet = command.mesh->materials[i]->descriptorSet->GetVkHandle();
            commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 1, &descriptorSet, VK_PIPELINE_BIND_POINT_GRAPHICS, 1);

            gRendererUtility().DrawPrimitive(commandBuffer, primitive);
         }
      }
   }

   RenderSkybox(commandBuffer);

   mRenderTarget->End(mVulkanApp->GetImageAvailableSemaphore(), mPhysicallyBasedRenderingComplete);

   // Todo: Should be in Engine somewhere
   gScreenQuadUi().Render(mVulkanApp);
}

void PhysicallyBasedRendering::Run()
{
   Utopian::gEngine().Run();
}

void PhysicallyBasedRendering::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   Utopian::gEngine().HandleMessages(hWnd, uMsg, wParam, lParam);
}

void PhysicallyBasedRendering::InitSkybox()
{
   Vk::Device* device = mVulkanApp->GetDevice();

   mSkybox.texture = Vk::gTextureLoader().LoadCubemapTexture("data/textures/environments/papermill.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);
   mSkybox.model = gModelLoader().LoadBox();

   Vk::EffectCreateInfo effectDesc;
   effectDesc.shaderDesc.vertexShaderPath = "data/shaders/skybox/skybox.vert";
   effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/skybox/skybox.frag";
   mSkybox.effect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, mRenderTarget->GetRenderPass(), effectDesc);

   mSkybox.inputBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
   mSkybox.shaderVariables.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

   mSkybox.effect->BindUniformBuffer("UBO_sharedVariables", mSkybox.shaderVariables);
   mSkybox.effect->BindUniformBuffer("UBO_input", mSkybox.inputBlock);
   //mSkybox.effect->BindCombinedImage("samplerCubeMap", *mSkybox.texture);
}

void PhysicallyBasedRendering::RenderSkybox(Vk::CommandBuffer* commandBuffer)
{
   mSkybox.shaderVariables.data.viewMatrix = mCamera->GetView();
   mSkybox.shaderVariables.data.projectionMatrix = mCamera->GetProjection();
   mSkybox.shaderVariables.data.inverseProjectionMatrix = glm::inverse(glm::mat3(mCamera->GetProjection()));
   mSkybox.shaderVariables.data.eyePos = glm::vec4(mCamera->GetPosition(), 1.0f);
   mSkybox.shaderVariables.data.mouseUV = gInput().GetMousePosition();
   mSkybox.shaderVariables.data.time = (float)gTimer().GetTime();
   mSkybox.shaderVariables.data.viewportSize = glm::vec2(mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());
   mSkybox.shaderVariables.UpdateMemory();

   mSkybox.inputBlock.data.world = glm::scale(glm::mat4(), glm::vec3(10000.0f));
   mSkybox.inputBlock.UpdateMemory();

   commandBuffer->CmdBindPipeline(mSkybox.effect->GetPipeline());
   commandBuffer->CmdBindDescriptorSets(mSkybox.effect);

   Primitive* primitive = mSkybox.model->GetPrimitive(0);
   commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());
   commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
   commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), 1, 0, 0, 0);
}

void PhysicallyBasedRendering::GenerateFilteredCubemaps()
{
   mIrradianceMap = FilterCubemap(64, VK_FORMAT_R32G32B32A32_SFLOAT, mSkybox.texture.get(),
                                  "C:/Git/UtopianEngine/source/demos/pbr/shaders/irradiance_filter.frag");

   mSpecularMap = FilterCubemap(512, VK_FORMAT_R16G16B16A16_SFLOAT, mSkybox.texture.get(),
                                "C:/Git/UtopianEngine/source/demos/pbr/shaders/specular_filter.frag");
}

SharedPtr<Vk::Texture> PhysicallyBasedRendering::FilterCubemap(uint32_t dimension, VkFormat format, Vk::Texture* inputCubemap,
                                                               std::string filterShader)
{
   const uint32_t numMipLevels = static_cast<uint32_t>(floor(log2(dimension))) + 1;
   Vk::Device* device = mVulkanApp->GetDevice();

   // Offscreen framebuffer
   SharedPtr<Vk::Image> offscreen = std::make_shared<Vk::ImageColor>(device, dimension,
      dimension, format, "Offscreen irradiance image");

   SharedPtr<Vk::RenderTarget> renderTarget = std::make_shared<Vk::RenderTarget>(device, dimension, dimension);
   renderTarget->AddWriteOnlyColorAttachment(offscreen);
   renderTarget->SetClearColor(1, 1, 1, 1);
   renderTarget->Create();

   Vk::EffectCreateInfo effectDesc;
   effectDesc.shaderDesc.vertexShaderPath = "C:/Git/UtopianEngine/source/demos/pbr/shaders/cubemap_filter.vert";
   effectDesc.shaderDesc.fragmentShaderPath = filterShader;
   SharedPtr<Vk::Effect> effect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderTarget->GetRenderPass(), effectDesc);

   effect->BindCombinedImage("samplerEnv", *inputCubemap);

   SharedPtr<Vk::Texture> outputCubemap = Vk::gTextureLoader().CreateCubemapTexture(format, dimension, dimension, numMipLevels);

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

   outputCubemap->GetImage().LayoutTransition(*commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

   for (uint32_t mipLevel = 0; mipLevel < numMipLevels; mipLevel++)
   {
      for (uint32_t face = 0; face < 6; face++)
      {
         renderTarget->BeginRenderPass();

         float viewportSize = dimension * std::pow(0.5f, mipLevel);
         commandBuffer->CmdSetViewPort(viewportSize, viewportSize);

         struct PushConsts {
            glm::mat4 mvp;
            float roughness;
         } pushConsts;

         glm::mat4 world = glm::scale(glm::mat4(), glm::vec3(10000.0f));
         pushConsts.mvp = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 20000.0f) * matrices[face] * world;
         pushConsts.roughness = (float)mipLevel / (float)(numMipLevels - 1);
         commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(PushConsts), &pushConsts.mvp);

         commandBuffer->CmdBindPipeline(effect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(effect);

         Primitive* primitive = mSkybox.model->GetPrimitive(0);
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
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, outputCubemap->GetImage().GetVkHandle(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

         offscreen->LayoutTransition(*commandBuffer, offscreen->GetFinalLayout());
      }
   }

   outputCubemap->GetImage().LayoutTransition(*commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

   renderTarget->EndDebugLabelAndQueries();
   commandBuffer->Flush();

   return outputCubemap;
}

Utopian::Model* PhysicallyBasedRendering::AddModel(std::string filename, glm::vec3 pos, glm::vec3 scale, glm::quat rotation)
{
   SceneNode sceneNode;
   sceneNode.model = gModelLoader().LoadModel(filename);
   sceneNode.worldMatrix = glm::translate(glm::mat4(1.0f), pos) *
                           glm::mat4(rotation) *
                           glm::scale(glm::mat4(1.0f), scale);
   mSceneNodes.push_back(sceneNode);

   return sceneNode.model.get();
}

ModelInspector::ModelInspector(Utopian::Model* model)
{
   this->model = model;
}

ModelInspector::~ModelInspector()
{
}

void ModelInspector::UpdateUi()
{
   if (ImGui::CollapsingHeader("Primitives", ImGuiTreeNodeFlags_DefaultOpen))
   {
      for (uint32_t index = 0; index < model->GetNumPrimitives(); index++)
      {
         Utopian::Primitive* primitive = model->GetPrimitive(index);

         if (ImGui::TreeNodeEx(primitive->GetDebugName().c_str()))
         {
            ImGui::Text("Num vertices: %u", primitive->GetNumVertices());
            ImGui::Text("Num indices: %u", primitive->GetNumIndices());
            ImGui::TreePop();
         }
      }
   }

   static uint32_t selectedMaterial = 0;
   if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
   {
      for (uint32_t index = 0; index < model->GetNumMaterials(); index++)
      {
         Utopian::Material* material = model->GetMaterial(index);

         ImGuiTreeNodeFlags flags = 0;
         if (selectedMaterial == index)
            flags = ImGuiTreeNodeFlags_Selected;

         if (ImGui::TreeNodeEx(material->name.c_str(), flags))
         {
            ImGui::TreePop();
         }

         if (ImGui::IsItemClicked())
         {
            selectedMaterial = index;
            ClearTextures();
            AddTextures(selectedMaterial);
         }
      }
   }

   if (ImGui::CollapsingHeader("Selected material", ImGuiTreeNodeFlags_DefaultOpen))
   {
      Material* material = model->GetMaterial(selectedMaterial);

      ImGui::Text("Name: %s", material->name.c_str());
      ImGui::ColorEdit4("Color", &material->properties->data.baseColorFactor.x);
      ImGui::SliderFloat("Metallic", &material->properties->data.metallicFactor, 0.0, 1.0f);
      ImGui::SliderFloat("Roughness", &material->properties->data.roughnessFactor, 0.0, 1.0f);
      ImGui::SliderFloat("Ambient occlusion", &material->properties->data.occlusionFactor, 0.0, 1.0f);
      material->properties->UpdateMemory();

      if (ImGui::CollapsingHeader("Textures"))
      {
         for (auto& textureInfo : textureInfos)
         {
            ImGui::Text(textureInfo.path.c_str());
            if (ImGui::ImageButton(textureInfo.textureId, ImVec2(64, 64)))
            {
               // Nothing
            }
         }
      }
   }
}

void ModelInspector::ClearTextures()
{
   for (auto& textureInfo : textureInfos)
   {
       gEngine().GetImGuiRenderer()->FreeTexture(textureInfo.textureId);
   }

   textureInfos.clear();
}

void ModelInspector::AddTextures(uint32_t materialIndex)
{
   Material* material = model->GetMaterial(materialIndex);
   Utopian::ImGuiRenderer* ui = gEngine().GetImGuiRenderer();

   textureInfos.push_back(TextureInfo(ui->AddImage(material->colorTexture->GetImage()), material->colorTexture->GetPath()));
   textureInfos.push_back(TextureInfo(ui->AddImage(material->normalTexture->GetImage()), material->normalTexture->GetPath()));
   textureInfos.push_back(TextureInfo(ui->AddImage(material->specularTexture->GetImage()), material->specularTexture->GetPath()));
   textureInfos.push_back(TextureInfo(ui->AddImage(material->metallicRoughnessTexture->GetImage()), material->metallicRoughnessTexture->GetPath()));
   textureInfos.push_back(TextureInfo(ui->AddImage(material->occlusionTexture->GetImage()), material->occlusionTexture->GetPath()));
}
