#include "core/renderer/RendererUtility.h"
#include "core/renderer/Im3dRenderer.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/Engine.h"
#include "core/Input.h"
#include "core/World.h"
#include "core/physics/Physics.h"
#include "core/renderer/Renderer.h"
#include "core/LuaManager.h"
#include "core/ScriptExports.h"
#include "core/AssetLoader.h"
#include "core/ActorFactory.h"
#include "core/Profiler.h"
#include "core/Log.h"
#include "vulkan/EffectManager.h"
#include "core/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/Debug.h"
#include "vulkan/VulkanApp.h"
#include "imgui/imgui.h"
#include <core/renderer/RenderSettings.h>

namespace Utopian
{
   Engine::Engine(Window* window, const std::string& appName)
      : mAppName(appName)
   {
      srand((unsigned int)time(NULL));

      Utopian::Vk::Debug::TogglePerformanceWarnings();
      Utopian::Vk::Debug::SetupDebugLayers();

      mWindow = window;
      mWindow->SetTitle(mAppName);

      gLog().Start();

      mVulkanApp = std::make_shared<Utopian::Vk::VulkanApp>(window);
      mVulkanApp->Prepare();

      UTO_LOG(mAppName);
   }
   
   Engine::~Engine()
   {
      // Vulkan resources cannot be destroyed when they are in use on the GPU
      while (!mVulkanApp->PreviousFrameComplete())
      {
      }

      // Call application destroy function
      mDestroyCallback();

      Vk::gShaderFactory().Destroy();
      Vk::gEffectManager().Destroy();
      Vk::gTextureLoader().Destroy();
      gModelLoader().Destroy();

      gTimer().Destroy();
      gInput().Destroy();
      gLuaManager().Destroy();
      gScreenQuadUi().Destroy();
      gProfiler().Destroy();
      gRendererUtility().Destroy();

      // Destroy all plugins
      for(auto& plugin : mPlugins)
         plugin->Destroy();

      mImGuiRenderer->GarbageCollect();
   }

   void Engine::StartModules()
   {
      UTO_LOG("Starting engine modules");

      Vk::Device* device = mVulkanApp->GetDevice();
      Vk::gEffectManager().Start();
      Vk::gTextureLoader().Start(device);
      gModelLoader().Start(device);
      Vk::gShaderFactory().Start(device);
      Vk::gShaderFactory().AddIncludeDirectory("data/shaders/include");
      Vk::gShaderFactory().AddIncludeDirectory("data/shaders/");

      gTimer().Start();
      gInput().Start();
      gLuaManager().Start();
      gProfiler().Start(mVulkanApp.get());
      gRendererUtility().Start();
      gScreenQuadUi().Start(mVulkanApp.get());

      mImGuiRenderer = std::make_shared<ImGuiRenderer>(mVulkanApp.get(), mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());

      for(auto& plugin : mPlugins)
         plugin->Start(this);

      for(auto& plugin : mPlugins)
         plugin->PostInit(this);

      UTO_LOG("Engine modules ready");
   }

   void Engine::Run()
   {
      while (true)
      {
         bool closeWindow = mWindow->DispatchMessages();

         if (!closeWindow)
         {
            Tick();
         }
         else
         {
            break;
         }
      }
   }

   void Engine::Tick()
   {
      Update();
      Render();

      gInput().Update(0);
   }

   void Engine::Update()
   {
      for(auto& plugin : mPlugins)
         plugin->NewFrame();

      mImGuiRenderer->NewFrame();

      for(auto& plugin : mPlugins)
         plugin->Update();

      gProfiler().Update();

      Vk::gEffectManager().Update();

      // Call the application Update() function
      mUpdateCallback();

      for(auto& plugin : mPlugins)
         plugin->EndFrame();

      mImGuiRenderer->EndFrame();
   }

   void Engine::Render()
   {
      if (mVulkanApp->PreviousFrameComplete())
      {
         mImGuiRenderer->GarbageCollect();

         if (mPreFrameCallback != nullptr)
            mPreFrameCallback();

         mVulkanApp->PrepareFrame();

         for(auto& plugin : mPlugins)
            plugin->Draw();

         mImGuiRenderer->Render();

         // Call the application Render() function
         mRenderCallback();

         mVulkanApp->Render();

         mVulkanApp->SubmitFrame();

         gTimer().CalculateFrameTime();
      }
   }
   
   void Engine::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
   {
      mVulkanApp->HandleMessages(hWnd, uMsg, wParam, lParam);

      gInput().HandleMessages(uMsg, wParam, lParam);

      switch (uMsg)
      {
      case WM_CLOSE:
         PostQuitMessage(0);
         break;
      default:
         DefWindowProc(hWnd, uMsg, wParam, lParam);
      }
   }

   void Engine::AddPlugin(SharedPtr<EnginePlugin> plugin)
   {
      mPlugins.push_back(plugin);
   }

   Vk::VulkanApp* Engine::GetVulkanApp()
   {
      return mVulkanApp.get();
   }

   ImGuiRenderer* Engine::GetImGuiRenderer()
   {
      return mImGuiRenderer.get();
   }

   Engine& gEngine()
   {
      return Engine::Instance();
   }

   DeferredRenderingPlugin::DeferredRenderingPlugin(const std::string& settingsFile)
   {
      mRenderingSettings = RenderingSettings();
      mSettingsFile = settingsFile;
   }

   void DeferredRenderingPlugin::Start(Engine* engine)
   {
      LoadSettingsFromFile();

      gRenderer().Start(engine->GetVulkanApp());
      gRenderer().SetUiOverlay(engine->GetImGuiRenderer());
      gRenderer().SetRenderingSettings(mRenderingSettings);

      // Todo: There is a dependency between loading the actors from Lua and the terrain creation
      // Terrain needs to be created before World::Instance().LoadScene();
      gRenderer().PostWorldInit();
   }

   void DeferredRenderingPlugin::PostInit(Engine* engine)
   {

   }

   void DeferredRenderingPlugin::Destroy()
   {
      gRenderer().Destroy();
   }

   void DeferredRenderingPlugin::Update()
   {
      gRenderer().Update();
   }

   void DeferredRenderingPlugin::Draw()
   {
      gRenderer().Render();
   }

   void DeferredRenderingPlugin::NewFrame()
   {
      gRenderer().NewUiFrame();
   }

   void DeferredRenderingPlugin::EndFrame()
   {
      gRenderer().EndUiFrame();
   }

   void DeferredRenderingPlugin::LoadSettingsFromFile()
   {
      gLuaManager().ExecuteFile(mSettingsFile.c_str());

      LuaPlus::LuaObject luaSettings = gLuaManager().GetLuaState()->GetGlobal("settings");
      if (luaSettings.IsNil())
         assert(0);

      mRenderingSettings.shadingMethod = ShadingMethod::PHONG;
      std::string shadingMethod = luaSettings["shadingMethod"].ToString();
      if (shadingMethod == "phong")
         mRenderingSettings.shadingMethod = ShadingMethod::PHONG;
      else if (shadingMethod == "pbr")
         mRenderingSettings.shadingMethod = ShadingMethod::PBR;

      mRenderingSettings.sky = luaSettings["sky"].ToString();
      mRenderingSettings.fogColor = glm::vec4(luaSettings["fogColor_r"].ToNumber(),
                                              luaSettings["fogColor_g"].ToNumber(),
                                              luaSettings["fogColor_b"].ToNumber(), 1.0f);
      mRenderingSettings.deferredPipeline = luaSettings["deferredPipeline"].GetBoolean();
      mRenderingSettings.fogStart = (float)luaSettings["fogStart"].ToNumber();
      mRenderingSettings.fogDistance = (float)luaSettings["fogDistance"].ToNumber();
      mRenderingSettings.ssaoRadius = (float)luaSettings["ssaoRadius"].ToNumber();
      mRenderingSettings.ssaoBias = (float)luaSettings["ssaoBias"].ToNumber();
      mRenderingSettings.blurRadius = (int)luaSettings["blurRadius"].ToInteger();
      mRenderingSettings.grassViewDistance = (float)luaSettings["grassViewDistance"].ToNumber();
      mRenderingSettings.blockViewDistance = (int)luaSettings["blockViewDistance"].ToInteger();
      mRenderingSettings.shadowsEnabled = luaSettings["shadowsEnabled"].GetBoolean();
      mRenderingSettings.normalMapping = luaSettings["normalMapping"].GetBoolean();
      mRenderingSettings.ssaoEnabled = luaSettings["ssaoEnabled"].GetBoolean();
      mRenderingSettings.ssrEnabled = luaSettings["ssrEnabled"].GetBoolean();
      mRenderingSettings.iblEnabled = luaSettings["iblEnabled"].GetBoolean();
      mRenderingSettings.bloomEnabled = luaSettings["bloomEnabled"].GetBoolean();
      mRenderingSettings.skyboxReflections = luaSettings["skyboxReflections"].GetBoolean();
      mRenderingSettings.waterEnabled = luaSettings["waterEnabled"].GetBoolean();
      mRenderingSettings.terrainEnabled = luaSettings["terrainEnabled"].GetBoolean();
      mRenderingSettings.fxaaEnabled = luaSettings["fxaaEnabled"].GetBoolean();
      mRenderingSettings.fxaaDebug = luaSettings["fxaaDebug"].GetBoolean();
      mRenderingSettings.godRaysEnabled = luaSettings["godRaysEnabled"].GetBoolean();
      mRenderingSettings.fxaaThreshold = (float)luaSettings["fxaaThreshold"].ToNumber();
      mRenderingSettings.shadowSampleSize = (int)luaSettings["shadowSampleSize"].ToInteger();
      mRenderingSettings.cascadeColorDebug = (float)luaSettings["cascadeColorDebug"].ToNumber();
      mRenderingSettings.cascadeSplitLambda = (float)luaSettings["cascadeSplitLambda"].ToNumber();
      mRenderingSettings.sunSpeed = (float)luaSettings["sunSpeed"].ToNumber();
      mRenderingSettings.sunInclination = (float)luaSettings["sunInclination"].ToNumber();
      mRenderingSettings.sunAzimuth = (float)luaSettings["sunAzimuth"].ToNumber();
      mRenderingSettings.tessellationFactor = (float)luaSettings["tessellationFactor"].ToNumber();
      mRenderingSettings.terrainTextureScaling = (float)luaSettings["terrainTextureScaling"].ToNumber();
      mRenderingSettings.terrainBumpmapAmplitude = (float)luaSettings["terrainBumpmapAmplitude"].ToNumber();
      mRenderingSettings.terrainWireframe = (float)luaSettings["terrainWireframe"].ToNumber();
      mRenderingSettings.tonemapping = (int)luaSettings["tonemapping"].ToInteger();
      mRenderingSettings.exposure = (float)luaSettings["exposure"].ToNumber();
      mRenderingSettings.bloomThreshold = (float)luaSettings["bloomThreshold"].ToNumber();
      mRenderingSettings.windStrength = (float)luaSettings["windStrength"].ToNumber();
      mRenderingSettings.windFrequency = (float)luaSettings["windFrequency"].ToNumber();
      mRenderingSettings.windEnabled = (float)luaSettings["windEnabled"].ToNumber();
      mRenderingSettings.numWaterCells = (int)luaSettings["numWaterCells"].ToInteger();
      mRenderingSettings.waterLevel = (float)luaSettings["waterLevel"].ToNumber();
      mRenderingSettings.waterColor = glm::vec3(luaSettings["waterColor_x"].ToNumber(),
                                                luaSettings["waterColor_y"].ToNumber(),
                                                luaSettings["waterColor_z"].ToNumber());
      mRenderingSettings.foamColor = glm::vec3(luaSettings["foamColor_x"].ToNumber(),
                                               luaSettings["foamColor_y"].ToNumber(),
                                               luaSettings["foamColor_z"].ToNumber());
      mRenderingSettings.waveSpeed = (float)luaSettings["waveSpeed"].ToNumber();
      mRenderingSettings.foamSpeed = (float)luaSettings["foamSpeed"].ToNumber();
      mRenderingSettings.waterDistortionStrength = (float)luaSettings["waterDistortionStrength"].ToNumber();
      mRenderingSettings.shorelineDepth = (float)luaSettings["shorelineDepth"].ToNumber();
      mRenderingSettings.waveFrequency = (float)luaSettings["waveFrequency"].ToNumber();
      mRenderingSettings.waterSpecularity = (float)luaSettings["waterSpecularity"].ToNumber();
      mRenderingSettings.waterTransparency = (float)luaSettings["waterTransparency"].ToNumber();
      mRenderingSettings.underwaterViewDistance = (float)luaSettings["underwaterViewDistance"].ToNumber();
   }

   void ECSPlugin::Start(Engine* engine)
   {
      gWorld().Start();
      gPhysics().Start();
      gAssetLoader().Start();
   }

   void ECSPlugin::PostInit(Engine* engine)
   {
      ActorFactory::LoadFromFile(engine->GetVulkanApp()->GetWindow(), "data/scene.lua");
      gWorld().LoadScene();

      ScriptExports::Register();
      ScriptImports::Register();
      //gLuaManager().ExecuteFile("data/scripts/procedural_assets.lua");
   }

   void ECSPlugin::Destroy()
   {
      gWorld().Destroy();
      gAssetLoader().Destroy();
      gPhysics().Destroy();
   }

   void ECSPlugin::Update()
   {
      gWorld().Update();
      gPhysics().Update();
   }

   void ECSPlugin::Draw()
   {
   }

   void ECSPlugin::NewFrame()
   {
   }

   void ECSPlugin::EndFrame()
   {
   }
}