#include <string>
#include <time.h>
#include "core/components/Actor.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CRigidBody.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/RenderSettings.h"
#include "core/World.h"
#include "core/Engine.h"
#include "core/Log.h"
#include "editor/Editor.h"
#include "Game.h"
#include "core/Window.h"
#include "vulkan/Debug.h"
#include "vulkan/ModelLoader.h"

Game::Game(Utopian::Window* window)
	: mWindow(window)
{
	Utopian::RenderingSettings renderingSettings;
	renderingSettings.terrainEnabled = false;
	renderingSettings.waterEnabled = false;

	Utopian::gEngine().Start(window, "Utopian Engine (v0.2)");
	Utopian::gEngine().AddPlugin(std::make_shared<Utopian::ECSPlugin>());
	Utopian::gEngine().AddPlugin(std::make_shared<Utopian::DeferredRenderingPlugin>(renderingSettings));
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
	SharedPtr<Utopian::Actor> actor = Utopian::Actor::Create("Floor");
	Utopian::CTransform* transform = actor->AddComponent<Utopian::CTransform>(glm::vec3(0.0f, 3300, 0.0f));
	Utopian::CRenderable* renderable = actor->AddComponent<Utopian::CRenderable>();
	Utopian::CRigidBody* rigidBody = actor->AddComponent<Utopian::CRigidBody>();

	// Needs to be called before PostInit() since CRigidBody calculates AABB from loaded model
	Utopian::Vk::StaticModel* model = Utopian::Vk::gModelLoader().LoadGrid(100, 32);
	renderable->SetModel(model);

	actor->PostInit();
	Utopian::World::Instance().SynchronizeNodeTransforms();

	// Must be called after PostInit() since it needs the Renderable component
	rigidBody->SetKinematic(true);
	rigidBody->SetCollisionShapeType(Utopian::CollisionShapeType::BOX);
}

void Game::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Utopian::gEngine().HandleMessages(hWnd, uMsg, wParam, lParam);
}