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
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/Debug.h"
#include "vulkan/VulkanApp.h"
#include "imgui/imgui.h"

namespace Utopian
{
	Engine::Engine(Window* window, const std::string& appName)
		: mAppName(appName)
	{
		mWindow = window;
		mWindow->SetTitle(mAppName);

		gLog().Start();

		mVulkanApp = std::make_shared<Utopian::Vk::VulkanApp>(window);
		mVulkanApp->Prepare();

		UTO_LOG(mAppName);
		UTO_LOG("Starting engine modules");

		StartModules();

		UTO_LOG("Engine modules ready");
	}
	
	Engine::~Engine()
	{
		// Vulkan handles cannot be destroyed when they are in use on the GPU
		while (!mVulkanApp->PreviousFrameComplete())
		{
		}

		// Call application destroy function
		mDestroyCallback();

		Vk::gShaderFactory().Destroy();
		Vk::gEffectManager().Destroy();
		Vk::gTextureLoader().Destroy();
		Vk::gModelLoader().Destroy();

		gTimer().Destroy();
		gWorld().Destroy();
		gInput().Destroy();
		gLuaManager().Destroy();
		gAssetLoader().Destroy();
		gScreenQuadUi().Destroy();
		gProfiler().Destroy();
		gRendererUtility().Destroy();
		gRenderer().Destroy();
		gPhysics().Destroy();
	}

	void Engine::StartModules()
	{
		Vk::Device* device = mVulkanApp->GetDevice();
		Vk::gEffectManager().Start();
		Vk::gTextureLoader().Start(device);
		Vk::gModelLoader().Start(device);
		Vk::gShaderFactory().Start(device);
		Vk::gShaderFactory().AddIncludeDirectory("data/shaders/include");

		gTimer().Start();
		gWorld().Start();
		gInput().Start();
		gPhysics().Start();
		gLuaManager().Start();
		gAssetLoader().Start();
		gProfiler().Start();
		gRendererUtility().Start();
		gScreenQuadUi().Start(mVulkanApp.get());
		gRenderer().Start(mVulkanApp.get());

		// Todo: There is a dependency between loading the actors from Lua and the terrain creation
		// Terrain needs to be created before World::Instance().LoadScene();
		gRenderer().PostWorldInit();

		ActorFactory::LoadFromFile(mVulkanApp->GetWindow(), "data/scene.lua");
		gWorld().LoadScene();
		gWorld().Update();

		ScriptExports::Register();
		ScriptImports::Register();
		//gLuaManager().ExecuteFile("data/scripts/procedural_assets.lua");
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
		gRenderer().NewUiFrame();

		gWorld().Update();
		gRenderer().Update();
		gPhysics().Update();
		gProfiler().Update();

		Vk::gEffectManager().Update();

		// Call the application Update() function
		mUpdateCallback();

		gRenderer().EndUiFrame();
	}

	void Engine::Render()
	{
		if (mVulkanApp->PreviousFrameComplete())
		{
			gRenderer().GarbageCollect();

			gTimer().FrameEnd();

			mVulkanApp->PrepareFrame();

			gRenderer().Render();

			// Call the application Render() function
			mRenderCallback();

			// Present to screen
			mVulkanApp->Render();

			mVulkanApp->SubmitFrame();
			gTimer().FrameBegin();
		}
	}
	
	void Engine::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		mVulkanApp->HandleMessages(hWnd, uMsg, wParam, lParam);

		gInput().HandleMessages(uMsg, wParam, lParam);
	}

	Engine& gEngine()
	{
		return Engine::Instance();
	}
}