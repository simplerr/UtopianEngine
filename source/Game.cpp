#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "core/terrain/Terrain.h"
#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "Input.h"
#include "imgui/imgui.h"

// Testing
#include "core/components/Actor.h"
#include "core/components/Component.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CLight.h"
#include "core/components/CCamera.h"
#include "core/components/CNoClip.h"
#include "core/components/COrbit.h"
#include "core/components/CPlayerControl.h"
#include "core/ObjectManager.h"
#include "core/World.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/EffectManager.h"
#include "vulkan/ScreenQuadUi.h"
#include "core/renderer/RenderingManager.h"
#include "core/renderer/RendererUtility.h"
#include "editor/Editor.h"
#include "utility/Utility.h"
#include "core/LuaManager.h"
#include "core/ActorFactory.h"
#include "core/AssetLoader.h"
#include "core/ScriptExports.h"
#include "LuaPlus.h"

using namespace Utopian;

namespace Utopian
{
	Game::Game(Window* window) 
		: mWindow(window)
	{
		srand(time(NULL));

		mIsClosing = false;

		Utopian::Vk::VulkanDebug::TogglePerformanceWarnings();

		mRenderer = std::make_shared<Vk::Renderer>();
		mRenderer->InitSwapchain(window);
		mRenderer->Prepare();
		mRenderer->SetClearColor(ColorRGB(47, 141, 255));

		InitScene();

		ObjectManager::Instance().PrintObjects();
	}

	Game::~Game()
	{
	}

	void Game::InitScene()
	{
		ObjectManager::Start();
		Timer::Start();
		World::Start();
		Input::Start();
		LuaManager::Start();
		AssetLoader::Start();
		Vk::ShaderFactory::Start(mRenderer->GetDevice());
		Vk::ShaderFactory::Instance().AddIncludeDirectory("data/shaders/include");
		ScreenQuadUi::Start(mRenderer.get());

		gLuaManager().ExecuteFile("data/scripts/procedural_assets.lua");

		ScriptExports::Register();
		ScriptImports::Register();
		Vk::EffectManager::Start();
		Vk::ModelLoader::Start(mRenderer->GetDevice());
		Vk::TextureLoader::Start(mRenderer->GetDevice());

		mRenderer->PostInitPrepare();

		Vk::StaticModel* model = Vk::gModelLoader().LoadModel("data/NatureManufacture Assets/Meadow Environment Dynamic Nature/Rocks/Rocks/Models/m_rock_01.FBX");

		RendererUtility::Start();
		RenderingManager::Start(mRenderer.get());

		ActorFactory::LoadFromFile(mWindow, "data/scene.lua");
		World::Instance().LoadScene();

		// Note: There are dependencies on the initialization order here
		mTerrain = std::make_shared<Terrain>(mRenderer->GetDevice(), gRenderingManager().GetMainCamera(), mRenderer->GetRenderPass());
		mTerrain->SetEnabled(false);
		RenderingManager::Instance().SetTerrain(mTerrain.get());
		
		World::Instance().Update();

		RenderingManager::Instance().PostWorldInit();
		ObjectManager::Instance().PrintObjects();

		// Note: Needs to be called after a camera have been added to the scene
		mEditor = std::make_shared<Editor>(mRenderer->GetUiOverlay(), gRenderingManager().GetMainCamera(), &World::Instance(), RenderingManager::Instance().GetTerrain());
	}

	void Game::Update()
	{
		mRenderer->BeginUiUpdate();

		World::Instance().Update();
		RenderingManager::Instance().Update();
		Vk::EffectManager::Instance().Update();
		mEditor->Update();

		mRenderer->EndUiUpdate();
	}

	void Game::Draw()
	{
		RenderingManager::Instance().Render();
		mEditor->Draw();
		gScreenQuadUi().Render(mRenderer.get());
		mRenderer->Render();
	}
	
	bool Game::IsClosing()
	{
		return mIsClosing;
	}

#if defined(_WIN32)
	void Game::RenderLoop()
	{
		MSG msg;

		while (true)
		{
			// Frame begin
			Timer::Instance().FrameBegin();

			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					break;
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			if (mRenderer != nullptr && IsClosing() == false)
			{
				mRenderer->PrepareFrame();

				Update();
				Draw();

				// Note: This must be called after Camera::Update()
				Input::Instance().Update(0);

				mRenderer->SubmitFrame();

				// Frame end
				auto fps = Timer::Instance().FrameEnd();

				// Only display fps when 1.0s have passed
				if (fps != -1)
				{
					std::stringstream ss;
					ss << "Utopian Engine (alpha) ";
					ss << "FPS: " << Timer::Instance().GetFPS();
					std::string windowTitle = ss.str();
					SetWindowText(mWindow->GetHwnd(), windowTitle.c_str());
				}
			}
		}
	}

#endif

	void Game::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (mRenderer != nullptr)
			mRenderer->HandleMessages(hWnd, uMsg, wParam, lParam);

		Input::Instance().HandleMessages(uMsg, wParam, lParam);

		switch (uMsg)
		{
		case WM_CLOSE:
			DestroyWindow(mWindow->GetHwnd());
			PostQuitMessage(0);
			mIsClosing = true;
			break;
		}
	}
}