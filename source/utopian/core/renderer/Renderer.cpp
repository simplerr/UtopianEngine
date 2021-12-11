#include <core/renderer/RendererUtility.h>
#include <glm/gtc/matrix_transform.hpp>
#include "core/renderer/Renderer.h"
#include "core/renderer/Renderable.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Image.h"
#include "core/Camera.h"
#include "core/renderer/Primitive.h"
#include "core/renderer/Light.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "core/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/Im3dRenderer.h"
#include "core/Terrain.h"
#include "core/AssetLoader.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/jobs/SSAOJob.h"
#include "core/renderer/jobs/BlurJob.h"
#include "core/renderer/jobs/ShadowJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/GrassJob.h"
#include "core/renderer/jobs/SkydomeJob.h"
#include "core/renderer/jobs/SunShaftJob.h"
#include "core/renderer/jobs/DebugJob.h"
#include "core/renderer/InstancingManager.h"
#include "core/ScriptExports.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "core/Log.h"
#include "utility/math/Helpers.h"
#include "utility/Timer.h"
#include "utility/Utility.h"
#include "core/Input.h"
#include "core/Engine.h"

#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <imgui/imgui.h>

namespace Utopian
{
   Renderer& gRenderer()
   {
      return Renderer::Instance();
   }

   Renderer::Renderer(Vk::VulkanApp* vulkanApp)
   {
      UTO_LOG("Initializing Renderer");

      mNextNodeId = 0;
      mMainCamera = nullptr;
      mVulkanApp = vulkanApp;
      mDevice = vulkanApp->GetDevice();

      mSceneInfo.directionalLight = nullptr;
      mSceneInfo.sharedVariables.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      mIm3dRenderer = std::make_shared<Im3dRenderer>(mVulkanApp, glm::vec2(mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight()));
      mInstancingManager = std::make_shared<InstancingManager>(this);
   }

   Renderer::~Renderer()
   {
      // Cannot rely on instance group being destroyed when going out of scope since that happens
      // after the call to GarbageCollect()
      ClearInstanceGroups();
   }

   void Renderer::PostWorldInit()
   {
      std::string sceneDirectory = Utopian::ExtractFileDirectory(Utopian::gEngine().GetSceneSource());

      if (mRenderingSettings.terrainEnabled)
      {
         mSceneInfo.terrain = std::make_shared<Terrain>(mDevice);
         mSceneInfo.terrain->LoadHeightmap(sceneDirectory + "heightmap.ktx");
         mSceneInfo.terrain->LoadBlendmap(sceneDirectory + "blendmap.ktx");
      }
      else
         mSceneInfo.terrain = nullptr;

      ScriptExports::SetTerrain(GetTerrain());

      mJobGraph = std::make_shared<JobGraph>(mVulkanApp, GetTerrain(), mDevice, mRenderingSettings);

      LoadInstancesFromFile(sceneDirectory + "instances.txt");
      BuildAllInstances();
   }

   void Renderer::Update(double deltaTime)
   {
      UpdateCascades();
      UpdateSun();

      if (mSceneInfo.terrain != nullptr)
         mSceneInfo.terrain->Update(deltaTime);

      mMainCamera->UpdateFrustum();
      mJobGraph->Update(deltaTime);

      UpdateUi();
   }

   void Renderer::UpdateUi()
   {
      ImGuiRenderer::BeginWindow("Utopian Engine (v0.4)", glm::vec2(10, 10), 350.0f,
                                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

      glm::vec3 pos = mMainCamera->GetPosition();
      glm::vec3 dir = mMainCamera->GetDirection();

      ImGuiRenderer::TextV(std::string("Vulkan " + GetDevice()->GetVulkanVersion().version).c_str());
      ImGuiRenderer::TextV("Time: %.2f", Timer::Instance().GetTime());
      ImGuiRenderer::TextV("Frametime: %.2f", Timer::Instance().GetAverageFrameTime());
      ImGuiRenderer::TextV("FPS: %.2f", Timer::Instance().GetAverageFps());
      ImGuiRenderer::TextV("Camera pos = (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
      ImGuiRenderer::TextV("Camera dir = (%.2f, %.2f, %.2f)", dir.x, dir.y, dir.z);
      ImGuiRenderer::TextV("Models: %u, Lights: %u", mSceneInfo.renderables.size(), mSceneInfo.lights.size());

      ImGuiRenderer::EndWindow();

      if (ImGuiRenderer::GetMode() == UI_MODE_EDITOR)
      {
         ImGuiRenderer::BeginWindow("Rendering settings", glm::vec2(10, 150), 300.0f);

         DisplayRenderSettings(mRenderingSettings, mSceneInfo.terrain.get());

         if (ImGui::CollapsingHeader("Debug"), ImGuiTreeNodeFlags_DefaultOpen)
         {
            static int debugChannel = JobGraph::DebugChannel::NONE;
            if (ImGui::Combo("Texture channel", &debugChannel, "None\0Position\0Normal\0Normal view space\0Albedo\0PBR"))
            {
               mJobGraph->SetDebugChannel((JobGraph::DebugChannel)debugChannel);
            }
         }

         mJobGraph->EnableJob(JobGraph::JobIndex::SSAO_INDEX, mRenderingSettings.ssaoEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::BLUR_INDEX, mRenderingSettings.ssaoEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::SSR_INDEX, mRenderingSettings.ssrEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::BLOOM_INDEX, mRenderingSettings.bloomEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::GEOMETRY_THICKNESS_INDEX, mRenderingSettings.ssrEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::WATER_INDEX, mRenderingSettings.waterEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::FRESNEL_INDEX, mRenderingSettings.waterEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::OPAQUE_COPY_INDEX, mRenderingSettings.waterEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::SHADOW_INDEX, mRenderingSettings.shadowsEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::SUN_SHAFT_INDEX, mRenderingSettings.godRaysEnabled);
         mJobGraph->EnableJob(JobGraph::JobIndex::DOF_INDEX, mRenderingSettings.dofEnabled);

         if (ImGui::Button("Dump memory statistics"))
         {
            mDevice->DumpMemoryStats("memory-statistics.json");
         }

         ImGuiRenderer::EndWindow();
      }
   }

   /*
      Calculate frustum split depths and matrices for the shadow map cascades
      Based on https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
      From Sascha Willems example demos.
   */
   void Renderer::UpdateCascades()
   {
      float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

      float nearClip = mMainCamera->GetNearPlane();
      float farClip = mMainCamera->GetFarPlane();
      float clipRange = farClip - nearClip;

      float minZ = nearClip;
      float maxZ = nearClip + clipRange;

      float range = maxZ - minZ;
      float ratio = maxZ / minZ;

      // Calculate split depths based on view camera furstum
      // Based on method presentd in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
      for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
      {
         float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
         float log = minZ * std::pow(ratio, p);
         float uniform = minZ + range * p;
         float d = mRenderingSettings.cascadeSplitLambda * (log - uniform) + uniform;
         cascadeSplits[i] = (d - nearClip) / clipRange;
      }

      // Calculate orthographic projection matrix for each cascade
      float lastSplitDist = 0.0;
      for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
      {
         float splitDist = cascadeSplits[i];

         glm::vec3 frustumCorners[8] = {
            glm::vec3(-1.0f,  1.0f, -1.0f),
            glm::vec3(1.0f,  1.0f, -1.0f),
            glm::vec3(1.0f, -1.0f, -1.0f),
            glm::vec3(-1.0f, -1.0f, -1.0f),
            glm::vec3(-1.0f,  1.0f,  1.0f),
            glm::vec3(1.0f,  1.0f,  1.0f),
            glm::vec3(1.0f, -1.0f,  1.0f),
            glm::vec3(-1.0f, -1.0f,  1.0f),
         };

         // Project frustum corners into world space
         glm::mat4 invCam = glm::inverse(mMainCamera->GetProjection() * mMainCamera->GetView());
         for (uint32_t i = 0; i < 8; i++) {
            glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
            frustumCorners[i] = invCorner / invCorner.w;
         }

         for (uint32_t i = 0; i < 4; i++) {
            glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
         }

         // Get frustum center
         glm::vec3 frustumCenter = glm::vec3(0.0f);
         for (uint32_t i = 0; i < 8; i++) {
            frustumCenter += frustumCorners[i];
         }
         frustumCenter /= 8.0f;

         float radius = 0.0f;
         for (uint32_t i = 0; i < 8; i++) {
            float distance = glm::length(frustumCorners[i] - frustumCenter);
            radius = glm::max(radius, distance);
         }
         radius = std::ceil(radius * 16.0f) / 16.0f;

         glm::vec3 maxExtents = glm::vec3(radius);
         glm::vec3 minExtents = -maxExtents;

         // Todo: Note: vec3(-1, 1, -1) is needed to make the shadows match phong shading
         glm::vec3 lightDir;
         if (mSceneInfo.directionalLight != nullptr)
            lightDir = glm::vec3(-1, 1, -1) * glm::normalize(mSceneInfo.directionalLight->GetDirection());
         else
            lightDir = glm::vec3(0.0f);

         glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

         // glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
         // Note: from Saschas examples the zNear was 0.0f, unclear why I need to set it to -(maxExtents.z - minExtents.z).
         glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -(maxExtents.z - minExtents.z), maxExtents.z - minExtents.z);

         // Store split distance and matrix in cascade
         mSceneInfo.cascades[i].splitDepth = (mMainCamera->GetNearPlane() + splitDist * clipRange) * -1.0f;
         mSceneInfo.cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;

         lastSplitDist = cascadeSplits[i];
      }
   }

   void Renderer::UpdateSun()
   {
      // Move sun
      mSceneInfo.sunInfo.azimuth += (float)Timer::Instance().GetTime() / 10000000 * mRenderingSettings.sunSpeed;

      // Calculate light direction
      float sunInclination = glm::radians(mRenderingSettings.sunInclination);

      mSceneInfo.sunInfo.direction = glm::vec3(sin(sunInclination) * cos(mSceneInfo.sunInfo.azimuth),
                                               cos(sunInclination),
                                               sin(sunInclination) * sin(mSceneInfo.sunInfo.azimuth));

      // Note: Negation of Z
      if (mSceneInfo.directionalLight != nullptr)
         mSceneInfo.directionalLight->SetDirection(glm::vec3(1, 1, -1) * mSceneInfo.sunInfo.direction);
   }

   void Renderer::Render()
   {
      // Note: This had to be done here due to the different periodicity of Update() and Render().
      // This should be corrected.
      mIm3dRenderer->UploadVertexData();

      // Deferred pipeline
      if (mRenderingSettings.deferredPipeline == true)
      {
         mSceneInfo.im3dVertices = mIm3dRenderer->GetVertexBuffer();
         mSceneInfo.sharedVariables.data.viewMatrix = mMainCamera->GetView();
         mSceneInfo.sharedVariables.data.projectionMatrix = mMainCamera->GetProjection();
         mSceneInfo.sharedVariables.data.inverseProjectionMatrix = glm::inverse(glm::mat3(mMainCamera->GetProjection()));
         mSceneInfo.sharedVariables.data.eyePos = glm::vec4(mMainCamera->GetPosition(), 1.0f);
         mSceneInfo.sharedVariables.data.mouseUV = gInput().GetMousePosition();
         mSceneInfo.sharedVariables.data.time = (float)gTimer().GetTime();
         mSceneInfo.sharedVariables.data.viewportSize = glm::vec2(mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());
         mSceneInfo.sharedVariables.UpdateMemory();

         mJobGraph->Render(mSceneInfo, mRenderingSettings);
      }

      gScreenQuadUi().Render(mVulkanApp);
   }

   void Renderer::NewUiFrame()
   {
      mIm3dRenderer->NewFrame();
   }

   void Renderer::EndUiFrame()
   {
      mIm3dRenderer->EndFrame();
   }

   void Renderer::AddRenderable(Renderable* renderable)
   {
      renderable->SetId(mNextNodeId++);
      mSceneInfo.renderables.push_back(renderable);
   }

   void Renderer::AddLight(Light* light)
   { 
      // Directional
      if (light->GetType() == 0)
      {
         mSceneInfo.directionalLight = light;
         mSceneInfo.directionalLight->SetDirection(glm::vec3(1, 1, -1) * mSceneInfo.sunInfo.direction);
      }

      light->SetId(mNextNodeId++);
      mSceneInfo.lights.push_back(light);
   }

   void Renderer::AddCamera(Camera* camera)
   {
      camera->SetId(mNextNodeId++);
      mSceneInfo.cameras.push_back(camera);
   }

   void Renderer::RemoveRenderable(Renderable* renderable)
   {
      for (auto iter = mSceneInfo.renderables.begin(); iter != mSceneInfo.renderables.end(); iter++)
      {
         // Note: No need to free memory here since that will happen when the SharedPtr is removed from the CRenderable
         if ((*iter)->GetId() == renderable->GetId())
         {
            mSceneInfo.renderables.erase(iter);
            break;
         }
      }
   }

   void Renderer::RemoveLight(Light* light)
   {
      for (auto iter = mSceneInfo.lights.begin(); iter != mSceneInfo.lights.end(); iter++)
      {
         if ((*iter)->GetId() == light->GetId())
         {
            mSceneInfo.lights.erase(iter);
            break;
         }
      }

      if (mSceneInfo.directionalLight == light)
         mSceneInfo.directionalLight = nullptr;
   }

   void Renderer::RemoveCamera(Camera* camera)
   {
      for (auto iter = mSceneInfo.cameras.begin(); iter != mSceneInfo.cameras.end(); iter++)
      {
         if ((*iter)->GetId() == camera->GetId())
         {
            mSceneInfo.cameras.erase(iter);
            break;
         }
      }
   }

   void Renderer::UpdateInstanceAltitudes()
   {
      mInstancingManager->UpdateInstanceAltitudes();
   }

   void Renderer::AddInstancedAsset(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, bool animated, bool castShadow)
   {
      mInstancingManager->AddInstancedAsset(assetId, position, rotation, scale, animated, castShadow);
   }

   void Renderer::RemoveInstancesWithinRadius(uint32_t assetId, glm::vec3 position, float radius)
   {
      mInstancingManager->RemoveInstancesWithinRadius(assetId, position, radius);
   }

   void Renderer::ClearInstanceGroups()
   {
      mInstancingManager->ClearInstanceGroups();
   }

   void Renderer::SaveInstancesToFile(const std::string& filename)
   {
      mInstancingManager->SaveInstancesToFile(filename);
   }

   void Renderer::LoadInstancesFromFile(const std::string& filename)
   {
      mInstancingManager->LoadInstancesFromFile(filename);
   }

   void Renderer::BuildAllInstances()
   {
      mInstancingManager->BuildAllInstances();
   }

   void Renderer::SetMainCamera(SharedPtr<Camera> camera)
   {
      mMainCamera = camera;
   }

   void Renderer::SetUiOverlay(ImGuiRenderer* imguiRenderer)
   {
      mImGuiRenderer = imguiRenderer;
   }

   Terrain* Renderer::GetTerrain() const
   {
      return mSceneInfo.terrain.get();
   }

   SceneInfo* Renderer::GetSceneInfo()
   {
      return &mSceneInfo;
   }

   const RenderingSettings& Renderer::GetRenderingSettings() const
   {
      return mRenderingSettings;
   }

   void Renderer::SetRenderingSettings(const RenderingSettings& renderingSettings)
   {
      mRenderingSettings = renderingSettings;
   }

   const SharedShaderVariables& Renderer::GetSharedShaderVariables() const
   {
      return mSceneInfo.sharedVariables;
   }

   Camera* Renderer::GetMainCamera() const
   {
      return mMainCamera.get();
   }

   Vk::Device* Renderer::GetDevice() const
   {
      return mDevice;
   }

   ImGuiRenderer* Renderer::GetUiOverlay()
   {
      return mImGuiRenderer;
   }

   uint32_t Renderer::GetWindowWidth() const
   {
      return mVulkanApp->GetWindowWidth();
   }

   uint32_t Renderer::GetWindowHeight() const
   {
      return mVulkanApp->GetWindowHeight();
   }
}
