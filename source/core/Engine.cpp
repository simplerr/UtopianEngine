#include "core/Engine.h"
#include "core/World.h"
#include "core/renderer/Renderer.h"
#include "core/LuaManager.h"
#include "core/ScriptExports.h"
#include "core/AssetLoader.h"
#include "core/ActorFactory.h"
#include "core/renderer/RendererUtility.h"
#include "vulkan/EffectManager.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/ScreenQuadUi.h"
#include "vulkan/Debug.h"
#include "Input.h"
#include "vulkan/VulkanApp.h"

namespace Utopian
{
	Engine::Engine(SharedPtr<Vk::VulkanApp> vulkanApp)
		: mVulkanApp(vulkanApp)
	{
		Vk::Debug::ConsolePrint("Starting engine modules...");

		StartModules();

		Vk::Debug::ConsolePrint("Engine modules ready");
	}
	
	Engine::~Engine()
	{

	}

	void Engine::StartModules()
	{
		Timer::Start();
		World::Start();
		Input::Start();
		LuaManager::Start();
		AssetLoader::Start();
		Vk::ShaderFactory::Start(mVulkanApp->GetDevice());
		Vk::ShaderFactory::Instance().AddIncludeDirectory("data/shaders/include");
		Vk::EffectManager::Start();
		ScreenQuadUi::Start(mVulkanApp.get());

		gLuaManager().ExecuteFile("data/scripts/procedural_assets.lua");

		ScriptExports::Register();
		ScriptImports::Register();
		Vk::ModelLoader::Start(mVulkanApp->GetDevice());
		Vk::TextureLoader::Start(mVulkanApp->GetDevice());

		mVulkanApp->PostInitPrepare();

		RendererUtility::Start();
		Renderer::Start(mVulkanApp.get());

		ActorFactory::LoadFromFile(mVulkanApp->GetWindow(), "data/scene.lua");
		World::Instance().LoadScene();
		World::Instance().Update();

		Renderer::Instance().PostWorldInit();
	}

	void Engine::Run()
	{
		while (true)
		{
			bool closeWindow = DispatchMessages();

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
		Timer::Instance().FrameBegin();
		mVulkanApp->PrepareFrame();

		Update();
		Render();

		Input::Instance().Update(0);

		mVulkanApp->SubmitFrame();
		auto fps = Timer::Instance().FrameEnd();
	}

	void Engine::Update()
	{
		mVulkanApp->BeginUiUpdate();

		World::Instance().Update();
		Renderer::Instance().Update();
		Vk::EffectManager::Instance().Update();

		// Call the applications Update() function
		mUpdateCallback();

		mVulkanApp->EndUiUpdate();
	}

	void Engine::Render()
	{
		Renderer::Instance().Render();
		gScreenQuadUi().Render(mVulkanApp.get());

		// Call the applications Render() function
		mRenderCallback();

		// Present to screen
		mVulkanApp->Render();
	}
	
	void Engine::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		mVulkanApp->HandleMessages(hWnd, uMsg, wParam, lParam);

		Utopian::Input::Instance().HandleMessages(uMsg, wParam, lParam);
	}

	bool Engine::DispatchMessages()
	{
		bool closeWindow = false;

		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
		 	{
				closeWindow = true;
		 	}
		 	else
		 	{
		 		TranslateMessage(&msg);
		 		DispatchMessage(&msg);
		 	}
		}

		return closeWindow;
	}

	Engine& gEngine()
	{
		return Engine::Instance();
	}
}