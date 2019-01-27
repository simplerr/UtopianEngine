#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/VulkanDebug.h"
#include <string>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include "core/World.h"
#include "core/renderer/RenderingManager.h"
#include "editor/Editor.h"
#include "core/Engine.h"

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
	mWindow->SetTitle("Utopian Engine (alpha)");
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