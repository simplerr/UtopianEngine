#include "PhysicallyBasedRendering.h"
#include "core/renderer/Model.h"
#include "core/glTFLoader.h"
#include <glm/matrix.hpp>
#include <string>
#include <time.h>
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
   AddModel("data/models/gltf/Fox/glTF/Fox.gltf", glm::vec3(0.0f), glm::vec3(0.01f));
   AddModel("data/models/gltf/FlightHelmet/glTF/FlightHelmet.gltf", glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(1.0f));
   // AddModel("data/models/gltf/DamagedHelmet/glTF/DamagedHelmet.gltf", glm::vec3(2.0f, 1.0f, 0.0f), glm::vec3(1.0f));
   
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
         material->properties->data.metallicFactor = (float)x / (float)max;
         material->properties->data.roughnessFactor = (float)y / (float)max;
         material->properties->data.ao = 0.5;
         material->properties->UpdateMemory();
      }
   }
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

   for (auto& sceneNode : mSceneNodes)
      sceneNode.model = nullptr;

   mVertexInputParameters.GetBuffer()->Destroy();
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

   mRenderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
   mRenderTarget->AddWriteOnlyColorAttachment(mOutputImage);
   mRenderTarget->AddWriteOnlyDepthAttachment(mDepthImage);
   mRenderTarget->SetClearColor(1, 1, 1, 1);
   mRenderTarget->Create();

   Vk::EffectCreateInfo effectDesc;
   effectDesc.shaderDesc.vertexShaderPath = "C:/Git/UtopianEngine/source/demos/pbr/shaders/pbr.vert";
   effectDesc.shaderDesc.fragmentShaderPath = "C:/Git/UtopianEngine/source/demos/pbr/shaders/pbr.frag";
   // Note: Not the default winding order
   effectDesc.pipelineDesc.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
   mEffect = Vk::Effect::Create(device, mRenderTarget->GetRenderPass(), effectDesc);

   effectDesc.shaderDesc.vertexShaderPath = "C:/Git/UtopianEngine/source/demos/pbr/shaders/pbr_skinning.vert";
   mSkinningEffect = Vk::Effect::Create(device, mRenderTarget->GetRenderPass(), effectDesc);

   mEffect->BindUniformBuffer("UBO_input", mVertexInputParameters);
   mSkinningEffect->BindUniformBuffer("UBO_input", mVertexInputParameters);

   InitSkybox();

   gScreenQuadUi().AddQuad(0, 0, width, height, mOutputImage.get(), mSampler.get());
}

void PhysicallyBasedRendering::UpdateCallback()
{
   glm::vec3 cameraPos = mCamera->GetPosition();

   Material* material = mSceneNodes[0].model->GetMaterial(0);

   ImGuiRenderer::BeginWindow("PBR Demo", glm::vec2(10, 150), 300.0f);
   ImGui::Text("Camera pos: (%.2f %.2f %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
   ImGui::ColorEdit4("Color", &material->properties->data.baseColorFactor.x);
   ImGui::SliderFloat("Metallic", &material->properties->data.metallicFactor, 0.0, 1.0f);
   ImGui::SliderFloat("Roughness", &material->properties->data.roughnessFactor, 0.0, 1.0f);
   ImGui::SliderFloat("Ambient occlusion", &material->properties->data.ao, 0.0, 1.0f);
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

   Material* material = mSceneNodes[0].model->GetMaterial(0);
   material->properties->UpdateMemory();

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
                                                VK_PIPELINE_BIND_POINT_GRAPHICS, 2);
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

   //mSkybox.texture = Vk::gTextureLoader().LoadCubemapTexture("data/textures/cubemap_space.ktx");
   //mSkybox.texture = Vk::gTextureLoader().LoadCubemapTexture("data/textures/environments/gcanyon_cube.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);
   mSkybox.texture = Vk::gTextureLoader().LoadCubemapTexture("data/textures/environments/papermill.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);
   mSkybox.model = gModelLoader().LoadBox();

   Vk::EffectCreateInfo effectDesc;
   effectDesc.shaderDesc.vertexShaderPath = "data/shaders/skybox/skybox.vert";
   effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/skybox/skybox.frag";
   effectDesc.pipelineDesc.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
   effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
   effectDesc.pipelineDesc.depthStencilState.depthWriteEnable = VK_FALSE;
   mSkybox.effect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, mRenderTarget->GetRenderPass(), effectDesc);

   mSkybox.inputBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
   mSkybox.shaderVariables.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

   mSkybox.effect->BindUniformBuffer("UBO_sharedVariables", mSkybox.shaderVariables);
   mSkybox.effect->BindUniformBuffer("UBO_input", mSkybox.inputBlock);
   mSkybox.effect->BindCombinedImage("samplerCubeMap", *mSkybox.texture);
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
