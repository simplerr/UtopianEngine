#include "RayTrace.h"
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

RayTrace::RayTrace(Utopian::Window* window)
   : mWindow(window)
{
   // Start Utopian Engine
   Utopian::gEngine().Start(window, "Raytrace demo");
   Utopian::gEngine().StartModules();
   Utopian::gEngine().RegisterUpdateCallback(&RayTrace::UpdateCallback, this);
   Utopian::gEngine().RegisterRenderCallback(&RayTrace::DrawCallback, this);
   Utopian::gEngine().RegisterDestroyCallback(&RayTrace::DestroyCallback, this);

   mVulkanApp = Utopian::gEngine().GetVulkanApp();

   mRayTraceComplete = std::make_shared<Vk::Semaphore>(mVulkanApp->GetDevice());
   //mVulkanApp->SetWaitSubmitSemaphore(mRayTraceComplete);

   InitResources();
}

RayTrace::~RayTrace()
{
   Utopian::gEngine().Destroy();
}

void RayTrace::DestroyCallback()
{
   // Free Vulkan resources
   mEffect = nullptr;
   mRayTraceComplete = nullptr;
   mOutputImage = nullptr;
   mSampler = nullptr;

   mInputParameters.GetBuffer()->Destroy();
   mSettingParameters.GetBuffer()->Destroy();
}

void RayTrace::InitResources()
{
   uint32_t width = mWindow->GetWidth();
   uint32_t height = mWindow->GetHeight();
   Vk::Device* device = mVulkanApp->GetDevice();

   mCamera = std::make_shared<MiniCamera>(glm::vec3(5, 25, 5), glm::vec3(25, 0, 25), 1, 2, 1.0f, width, height);

   Vk::EffectCreateInfo effectDesc;
   effectDesc.shaderDesc.computeShaderPath = "source/demos/raytracing/raytrace.comp";
   mEffect = Vk::Effect::Create(device, nullptr, effectDesc);

   mInputParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
   mSettingParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

   mOutputImage = std::make_shared<Vk::ImageStorage>(device, width, height, 1, "Raytrace image");
   mSampler = std::make_shared<Vk::Sampler>(device);

   mEffect->BindImage("outputImage", *mOutputImage);
   mEffect->BindUniformBuffer("UBO_input", mInputParameters);
   mEffect->BindUniformBuffer("UBO_settings", mSettingParameters);

   gScreenQuadUi().AddQuad(0, 0, width, height, mOutputImage.get(), mSampler.get());

   mSettingParameters.data.maxTraceDepth = 4;
}

void RayTrace::UpdateCallback()
{
   ImGuiRenderer::BeginWindow("Raytracing Demo", glm::vec2(10, 150), 300.0f);
   ImGui::SliderInt("Max trace depth", &mSettingParameters.data.maxTraceDepth, 1, 8);
   ImGuiRenderer::EndWindow();

   // Recompile shaders
   if (gInput().KeyPressed('R'))
   {
      Vk::gEffectManager().RecompileModifiedShaders();
   }

   mCamera->Update();
}

void RayTrace::DrawCallback()
{
   // Update uniforms
   CalculateRays();
   mInputParameters.data.eye = glm::vec4(mCamera->GetPosition(), 1.0f);
   mInputParameters.UpdateMemory();
   mSettingParameters.UpdateMemory();

   // Test rendering
   Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   commandBuffer.Begin();
   commandBuffer.CmdBindPipeline(mEffect->GetPipeline());
   commandBuffer.CmdBindDescriptorSets(mEffect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
   commandBuffer.CmdDispatch(mWindow->GetWidth() / 16, mWindow->GetHeight() / 16, 1);
   commandBuffer.Flush();

   // Todo: Should be in Engine somewhere
   gScreenQuadUi().Render(mVulkanApp);
}

void RayTrace::CalculateRays()
{
   glm::mat4 inverseViewProjection = glm::inverse(mCamera->GetProjection() * mCamera->GetView());

   glm::vec4 cameraPos = glm::vec4(mCamera->GetPosition(), 0.0f);
   glm::vec4 ray00 = inverseViewProjection * glm::vec4(-1, -1, 0, 1);
   ray00 = ray00 / ray00.w;
   ray00 -= cameraPos;

   glm::vec4 ray10 = inverseViewProjection * glm::vec4(+1, -1, 0, 1);
   ray10 = ray10 / ray10.w;
   ray10 -= cameraPos;

   glm::vec4 ray01 = inverseViewProjection * glm::vec4(-1, +1, 0, 1);
   ray01 = ray01 / ray01.w;
   ray01 -= cameraPos;

   glm::vec4 ray11 = inverseViewProjection * glm::vec4(+1, +1, 0, 1);
   ray11 = ray11 / ray11.w;
   ray11 -= cameraPos;

   mInputParameters.data.ray00 = ray00;
   mInputParameters.data.ray01 = ray01;
   mInputParameters.data.ray10 = ray10;
   mInputParameters.data.ray11 = ray11;
}

void RayTrace::Run()
{
   Utopian::gEngine().Run();
}

void RayTrace::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   Utopian::gEngine().HandleMessages(hWnd, uMsg, wParam, lParam);
}