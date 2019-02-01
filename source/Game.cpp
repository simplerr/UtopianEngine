#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/Debug.h"
#include <string>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include "core/World.h"
#include "core/renderer/Renderer.h"
#include "editor/Editor.h"
#include "core/Engine.h"

Game::Game(Utopian::Window* window) 
	: mWindow(window)
{
	srand(time(NULL));

	mIsClosing = false;

	Utopian::Vk::Debug::TogglePerformanceWarnings();
	Utopian::Vk::Debug::SetupDebugLayers();
	Utopian::Vk::Debug::ConsolePrint(mAppName);

	mVulkanApp = std::make_shared<Utopian::Vk::VulkanApp>(window);
	mVulkanApp->Prepare();
	
	// Start Utopian Engine
	Utopian::gEngine().Start(mVulkanApp);
	Utopian::gEngine().RegisterUpdateCallback(&Game::Update, this);
	Utopian::gEngine().RegisterRenderCallback(&Game::Draw, this);

	InitScene();

	// Note: Needs to be called after a camera have been added to the scene
	mEditor = std::make_shared<Utopian::Editor>(mVulkanApp->GetUiOverlay(), Utopian::gRenderer().GetMainCamera(), &Utopian::World::Instance(), Utopian::Renderer::Instance().GetTerrain());
	mWindow->SetTitle(mAppName);
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