#include <string>
#include <time.h>
#include "core/components/Actor.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CRigidBody.h"
#include "core/components/CPolyMesh.h"
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
#include "vulkan/TextureLoader.h"
#include "core/renderer/Im3dRenderer.h"

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
	Utopian::gEngine().RegisterPreFrameCallback(&Game::PreFrameCallback, this);

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

	Im3d::SetSize(3.0f);
	Im3d::DrawLine(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 5.0f, Im3d::Color_Red);
	Im3d::DrawLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 5.0f, Im3d::Color_Blue);
	Im3d::DrawPoint(glm::vec3(0.0f, 0.0f, 0.0f), 20.0f, Im3d::Color_Yellow);
}

void Game::DrawCallback()
{
	mEditor->Draw();
}

void Game::PreFrameCallback()
{
	mEditor->PreFrame();
}

void Game::Run()
{
	Utopian::gEngine().Run();
}

void Game::InitScene()
{
	AddGround();
}

void Game::AddGround()
{
	SharedPtr<Utopian::Actor> actor = Utopian::Actor::Create("Ground");
	Utopian::CTransform* transform = actor->AddComponent<Utopian::CTransform>(glm::vec3(0.0f, 0.0f, 0.0f));
	Utopian::CRenderable* renderable = actor->AddComponent<Utopian::CRenderable>();
	Utopian::CRigidBody* rigidBody = actor->AddComponent<Utopian::CRigidBody>();

	// Needs to be called before PostInit() since CRigidBody calculates AABB from loaded model
	auto model = Utopian::Vk::gModelLoader().LoadGrid(100, 2);
	renderable->SetModel(model);
	renderable->SetTileFactor(glm::vec2(50.0f));
	renderable->SetTexture(Utopian::Vk::gTextureLoader().LoadTexture("data/textures/prototype/Light/texture_12.ktx"));

	actor->PostInit();
	Utopian::World::Instance().SynchronizeNodeTransforms();

	// Must be called after PostInit() since it needs the Renderable component
	rigidBody->SetKinematic(true);
}

void Game::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Utopian::gEngine().HandleMessages(hWnd, uMsg, wParam, lParam);
}