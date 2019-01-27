#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "core/terrain/Terrain.h"
#include "vulkan/VulkanApp.h"
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
#include "core/Engine.h"
#include "LuaPlus.h"

Game::Game(Utopian::Window* window) 
	: mWindow(window)
{
	srand(time(NULL));

	mIsClosing = false;

	Utopian::Vk::VulkanDebug::TogglePerformanceWarnings();

	mVulkanApp = std::make_shared<Utopian::Vk::VulkanApp>(window);
	mVulkanApp->Prepare();
	
	// Start Utopian Engine
	Utopian::gEngine().Start(mVulkanApp);
	Utopian::gEngine().RegisterUpdateCallback(&Game::Update, this);
	Utopian::gEngine().RegisterRenderCallback(&Game::Draw, this);

	InitScene();

	// Note: Needs to be called after a camera have been added to the scene
	mEditor = std::make_shared<Utopian::Editor>(mVulkanApp->GetUiOverlay(), Utopian::gRenderingManager().GetMainCamera(), &Utopian::World::Instance(), Utopian::RenderingManager::Instance().GetTerrain());

	std::stringstream ss;
	ss << "Utopian Engine (alpha) ";
	std::string windowTitle = ss.str();
	SetWindowText(mWindow->GetHwnd(), windowTitle.c_str());
}

Game::~Game()
{
}

void Game::InitScene()
{

}

void Game::Update()
{
	mEditor->Update();
}

void Game::Draw()
{
	mEditor->Draw();
}

bool Game::IsClosing()
{
	return mIsClosing;
}

void Game::Run()
{
	Utopian::gEngine().Run();
}

void Game::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Utopian::gEngine().HandleMessages(hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(mWindow->GetHwnd());
		PostQuitMessage(0);
		mIsClosing = true;
		break;
	}
}