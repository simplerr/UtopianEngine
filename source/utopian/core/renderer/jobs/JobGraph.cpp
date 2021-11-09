#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/jobs/BloomJob.h"
#include "core/renderer/jobs/BlurJob.h"
#include "core/renderer/jobs/DebugJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/FXAAJob.h"
#include "core/renderer/jobs/FresnelJob.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/jobs/GBufferTerrainJob.h"
#include "core/renderer/jobs/GeometryThicknessJob.h"
#include "core/renderer/jobs/GrassJob.h"
#include "core/renderer/jobs/Im3dJob.h"
#include "core/renderer/jobs/JobGraph.h"
#include "core/renderer/jobs/OpaqueCopyJob.h"
#include "core/renderer/jobs/PixelDebugJob.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/jobs/SSAOJob.h"
#include "core/renderer/jobs/SSRJob.h"
#include "core/renderer/jobs/ShadowJob.h"
#include "core/renderer/jobs/SkyboxJob.h"
#include "core/renderer/jobs/SkydomeJob.h"
#include "core/renderer/jobs/SunShaftJob.h"
#include "core/renderer/jobs/TonemapJob.h"
#include "core/renderer/jobs/WaterJob.h"
#include "core/renderer/jobs/AtmosphereJob.h"
#include "core/renderer/jobs/OutlineJob.h"
#include "core/renderer/jobs/DepthOfFieldJob.h"
#include "core/Log.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Image.h"
#include <thread>
#include <algorithm>

namespace Utopian
{
   JobGraph::JobGraph(Vk::VulkanApp* vulkanApp, Terrain* terrain, Vk::Device* device, const RenderingSettings& renderingSettings)
   {
      Timestamp start = gTimer().GetTimestamp();

      uint32_t width = vulkanApp->GetWindowWidth();
      uint32_t height = vulkanApp->GetWindowHeight();

      /* Create the G-buffer attachments */
      mGBuffer.positionImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R32G32B32A32_SFLOAT, "G-buffer position image");
      mGBuffer.normalImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "G-buffer normal (world) image");
      mGBuffer.normalViewImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R8G8B8A8_UNORM, "G-buffer normal (view) image");
      mGBuffer.albedoImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "G-buffer albedo image");
      mGBuffer.depthImage = std::make_shared<Vk::ImageDepth>(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT, "G-buffer depth image");
      mGBuffer.specularImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "G-buffer specular image");
      mGBuffer.pbrImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "G-buffer PBR image");

      /* Create the main offscreen render target */
      mGBuffer.mainImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R32G32B32A32_SFLOAT, "Main image");

      /* Add jobs */
      GBufferTerrainJob* gbufferTerrainJob = new GBufferTerrainJob(device, terrain, width, height);
      gbufferTerrainJob->SetWaitSemaphore(vulkanApp->GetImageAvailableSemaphore());
      AddJob(gbufferTerrainJob);

      AddJob(new GBufferJob(device, width, height));
      AddJob(new SSAOJob(device, width / 2, height / 2));
      AddJob(new BlurJob(device, width / 2, height / 2));
      AddJob(new ShadowJob(device, width, height));
      AddJob(new DeferredJob(device, width, height));

      if (renderingSettings.sky == SKY_DOME)
         AddJob(new SkydomeJob(device, width, height));
      else if (renderingSettings.sky == SKY_BOX)
         AddJob(new SkyboxJob(device, width, height));
      else
         AddJob(new AtmosphereJob(device, width, height));

      AddJob(new SunShaftJob(device, width, height));
      AddJob(new OpaqueCopyJob(device, width, height));
      AddJob(new GeometryThicknessJob(device, width, height));
      AddJob(new WaterJob(device, width, height));
      AddJob(new SSRJob(device, width, height));
      AddJob(new FresnelJob(device, width, height));
      AddJob(new DebugJob(device, width, height));
      AddJob(new Im3dJob(device, width, height));
      AddJob(new BloomJob(device, width, height));
      AddJob(new DepthOfFieldJob(device, width, height));
      AddJob(new OutlineJob(device, width, height));
      AddJob(new TonemapJob(device, width, height));
      //AddJob(new PixelDebugJob(device, width, height));

      FXAAJob* fxaaJob = new FXAAJob(device, width, height);
      vulkanApp->SetWaitSubmitSemaphore(fxaaJob->GetCompletedSemahore());
      AddJob(fxaaJob);

      for(auto job : mJobs)
      {
         job->Init(mJobs, mGBuffer);
      }

      AsynchronousResourceLoading();

      for(auto job : mJobs)
      {
         job->PostInit(mJobs, mGBuffer);
      }

      /* Add debug render targets */
      ImGuiRenderer* imGuiRenderer = gRenderer().GetUiOverlay();
      mDebugDescriptorSets.position = imGuiRenderer->AddImage(*mGBuffer.positionImage);
      mDebugDescriptorSets.normal = imGuiRenderer->AddImage(*mGBuffer.normalImage);
      mDebugDescriptorSets.normalView = imGuiRenderer->AddImage(*mGBuffer.normalViewImage);
      mDebugDescriptorSets.albedo = imGuiRenderer->AddImage(*mGBuffer.albedoImage);
      mDebugDescriptorSets.pbr = imGuiRenderer->AddImage(*mGBuffer.pbrImage);

      //EnableJob(JobGraph::JobIndex::PIXEL_DEBUG_INDEX, false);

      UTO_LOG("Create JobGraph elapsed time: " + std::to_string(gTimer().GetElapsedTime(start)));
   }

   JobGraph::~JobGraph()
   {
      for(auto job : mJobs)
      {
         delete job;
      }
   }

   void JobGraph::AsynchronousResourceLoading()
   {
      // Splits the resource loading work from all jobs in the graph evenly
      // across the number of threads configured for asyncrhonous loading.
      auto work = [&](int currentThread, int numThreads)
      {
         uint32_t functionsInThreads = mJobs.size() / numThreads;
         uint32_t functionsInCurrentThread = functionsInThreads;
         if (currentThread == numThreads - 1)
            functionsInCurrentThread = mJobs.size() - ((numThreads - 1) * functionsInThreads);

         for (uint32_t j = 0; j < functionsInCurrentThread; j++)
         {
            mJobs[currentThread * functionsInThreads + j]->LoadResources();
         }
      };

      // From testing 4 threads shows the best performance.
      const uint32_t numThreads = 4;
      std::vector<std::thread> workerThreads;

      for (uint32_t i = 0; i < numThreads; i++)
      {
         workerThreads.push_back(std::thread(work, i, numThreads));
      }

      // Wait for all shader compilation worker threads to finish
      std::for_each(workerThreads.begin(), workerThreads.end(), [](std::thread& t) { t.join(); });
   }

   void JobGraph::Render(const SceneInfo& sceneInfo, const RenderingSettings& renderingSettings)
   {
      JobInput jobInput(sceneInfo, mJobs, renderingSettings);

      for (auto& job : mJobs)
      {
         job->PreRender(jobInput);
      }

      for (auto& job : mJobs)
      {
         job->Render(jobInput);
      }
   }

   void JobGraph::Update()
   {
      for (auto& job : mJobs)
      {
         job->Update();
      }

      // Fullscreen debug texture
      if (mDebugChannel != DebugChannel::NONE)
      {
         static ImTextureID textureId = mDebugDescriptorSets.position;
         switch (mDebugChannel)
         {
            case DebugChannel::POSITION: textureId = mDebugDescriptorSets.position; break;
            case DebugChannel::NORMAL: textureId = mDebugDescriptorSets.normal; break;
            case DebugChannel::NORMAL_VIEW: textureId = mDebugDescriptorSets.normalView; break;
            case DebugChannel::ALBEDO: textureId = mDebugDescriptorSets.albedo; break;
            case DebugChannel::PBR: textureId = mDebugDescriptorSets.pbr; break;
         }

         ImVec2 pos = ImVec2(0, 0);
         ImVec2 maxPos = ImVec2(pos.x + ImGui::GetWindowSize().x, pos.y + ImGui::GetWindowSize().y);
         ImGui::GetWindowDrawList()->AddImage(textureId, ImVec2(pos.x, pos.y),
                                              ImVec2(maxPos), ImVec2(0, 0), ImVec2(1, 1));
      }

      if (ImGuiRenderer::GetMode() == UI_MODE_EDITOR)
      {
         ImGuiRenderer::BeginWindow("Render targets:", glm::vec2(300.0f, 10.0f), 400.0f);

         ImVec2 textureSize = ImVec2(256, 256);
         ImGui::BeginGroup();
         ImGui::Text("Position");
         ImGui::Image(mDebugDescriptorSets.position, textureSize);
         ImGui::EndGroup();

         ImGui::SameLine();

         ImGui::BeginGroup();
         ImGui::Text("Normal");
         ImGui::Image(mDebugDescriptorSets.normal, textureSize);
         ImGui::EndGroup();

         ImGui::BeginGroup();
         ImGui::Text("Normal view space");
         ImGui::Image(mDebugDescriptorSets.normalView, textureSize);
         ImGui::EndGroup();

         ImGui::SameLine();

         ImGui::BeginGroup();
         ImGui::Text("Albedo");
         ImGui::Image(mDebugDescriptorSets.albedo, textureSize);
         ImGui::EndGroup();

         ImGuiRenderer::EndWindow();
      }
   }

   void JobGraph::SetDebugChannel(DebugChannel debugChannel)
   {
      mDebugChannel = debugChannel;
   }

   void JobGraph::AddJob(BaseJob* job)
   {
      // Setup synchronization dependecy to the previous job
      if (mJobs.size() > 0)
      {
         SharedPtr<Vk::Semaphore>& waitSemaphore = mJobs.back()->GetCompletedSemahore();
         job->SetWaitSemaphore(waitSemaphore);
      }

      mJobs.push_back(job);
   }

   void JobGraph::EnableJob(JobIndex jobIndex, bool enabled)
   {
      assert(jobIndex < mJobs.size());

      mJobs[jobIndex]->SetEnabled(enabled);
   }

   const GBuffer& JobGraph::GetGBuffer() const
   {
      return mGBuffer;
   }
}