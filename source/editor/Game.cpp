#include <string>
#include <time.h>
#include "core/renderer/Renderer.h"
#include "core/World.h"
#include "core/Engine.h"
#include "core/Log.h"
#include "editor/Editor.h"
#include "Game.h"
#include "core/Window.h"
#include "vulkan/Debug.h"

Game::Game(Utopian::Window* window)
	: mWindow(window)
{
	srand(time(NULL));

	Utopian::Vk::Debug::TogglePerformanceWarnings();
	Utopian::Vk::Debug::SetupDebugLayers();

	// Start Utopian Engine
	Utopian::gEngine().Start(window, "Utopian Engine (v0.2)");
	Utopian::gEngine().AddPlugin(std::make_shared<Utopian::ECSPlugin>());
	Utopian::gEngine().AddPlugin(std::make_shared<Utopian::DeferredRenderingPlugin>());
	Utopian::gEngine().StartModules();

	Utopian::gEngine().RegisterUpdateCallback(&Game::UpdateCallback, this);
	Utopian::gEngine().RegisterRenderCallback(&Game::DrawCallback, this);
	Utopian::gEngine().RegisterDestroyCallback(&Game::DestroyCallback, this);

	InitScene();

	// Note: Needs to be called after a camera have been added to the scene
	mEditor = std::make_shared<Utopian::Editor>(Utopian::gRenderer().GetUiOverlay(),
												Utopian::gRenderer().GetMainCamera(),
												&Utopian::World::Instance(),
												Utopian::Renderer::Instance().GetTerrain());
}

Game::~Game()
{
	Utopian::gEngine().Destroy();
}

void Game::DestroyCallback()
{
	mEditor = nullptr;
}

void Game::UpdateCallback()
{
	mEditor->Update();
}

void Game::DrawCallback()
{
	mEditor->Draw();
}

void Game::Run()
{
	Utopian::gEngine().Run();
}

void Game::InitScene()
{
	//SharedPtr<Utopian::Actor> actor = Utopian::Actor::Create("Sponza");
	// Utopian::CTransform* transform = actor->AddComponent<Utopian::CTransform>();
	// Utopian::CRenderable* renderable = actor->AddComponent<Utopian::CRenderable>();
	// renderable->LoadModel("data/models/sponza/sponza.obj");
}

void Game::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Utopian::gEngine().HandleMessages(hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(mWindow->GetHwnd());
		PostQuitMessage(0);
		break;
	}
}