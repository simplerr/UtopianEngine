#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "Terrain.h"
#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include "vulkan/ModelLoader.h"
#include "ecs/SystemManager.h"
#include "ecs/Entity.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/PhysicsComponent.h"
#include "ecs/components/HealthComponent.h"
#include "ecs/systems/RenderSystem.h"
#include "ecs/systems/PhysicsSystem.h"
#include "ecs/systems/HealthSystem.h"
#include "ecs/systems/EditorSystem.h"
#include "Input.h"

// Testing
#include "scene/SceneEntity.h"
#include "scene/SceneComponent.h"
#include "scene/CTransform.h"
#include "scene/CRenderable.h"
#include "scene/CLight.h"
#include "scene/CCamera.h"
#include "scene/CNoClip.h"
#include "scene/COrbit.h"
#include "scene/ObjectManager.h"
#include "scene/World.h"
#include "scene/SceneRenderer.h"

using namespace Scene;

namespace Vulkan
{
	Game::Game(Window* window) 
		: mWindow(window)
	{
		srand(time(NULL));

		mIsClosing = false;

		Vulkan::VulkanDebug::TogglePerformanceWarnings();

		mRenderer = make_shared<Renderer>();
		mRenderer->InitSwapchain(window);
		mRenderer->Prepare();

		mTerrain = make_shared<Terrain>(mRenderer.get());

		InitScene();

		ObjectManager::Instance().PrintObjects();
	}

	Game::~Game()
	{

	}

	void Game::InitScene()
	{
		ObjectManager::Start();
		World::Start();
		Input::Start();
		SceneRenderer::Start(mRenderer.get());
		SceneRenderer::Instance().SetTerrain(mTerrain.get());
		
		// Add camera
		auto cameraEntity = SceneEntity::Create("Camera");
		cameraEntity->AddComponent<CTransform>(vec3(67001.0f, 10300.0f, 68000.0f));
		CCamera* camera = cameraEntity->AddComponent<CCamera>(mWindow, 60.0f, 10.0f, 256000.0f);
		camera->SetMainCamera();

		cameraEntity->AddComponent<CNoClip>(300.0f);
		COrbit* orbit = cameraEntity->AddComponent<COrbit>(0.01f);
		orbit->SetTarget(vec3(81000.0f, 5300.0f, 78000.0f));

		// Add teapot
		auto teapot = SceneEntity::Create("Teapot");

		CTransform* transform = teapot->AddComponent<CTransform>(vec3(81000.0f, 5300.0f, 78000.0f));
		transform->SetScale(vec3(50.0f));

		CRenderable* mesh = teapot->AddComponent<CRenderable>();
		mesh->SetModel(mRenderer->mModelLoader->LoadModel(mRenderer->GetDevice(), "data/models/teapot.obj"));

		// Add teapot #2
		teapot = SceneEntity::Create("Teapot");

		transform = teapot->AddComponent<CTransform>(vec3(81000.0f, 5300.0f, 78000.0f));
		transform->SetScale(vec3(150.0f));

		orbit = teapot->AddComponent<COrbit>(-0.01f);
		orbit->SetTarget(vec3(81000.0f, 5300.0f, 78000.0f));

		mesh = teapot->AddComponent<CRenderable>();
		mesh->SetModel(mRenderer->mModelLoader->LoadModel(mRenderer->GetDevice(), "data/models/teapot.obj"));

		// Add light
		auto light = SceneEntity::Create("Light");

		//light->AddComponent<CTransform>(vec3(87000, 5000, 67000));
		light->AddComponent<CTransform>(vec3(0, 0.5, 0));

		CLight* lightComponent = light->AddComponent<CLight>();
		lightComponent->SetMaterial(vec4(1, 1, 1, 1));
		lightComponent->SetMaterials(vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), vec4(0, 0, 1, 1));
		lightComponent->SetDirection(vec3(1, -1, 1));
		lightComponent->SetAtt(0, 1, 0);
		lightComponent->SetIntensity(0.1f, 1.0f, 1.0f);
		lightComponent->SetType(Vulkan::LightType::DIRECTIONAL_LIGHT);
		lightComponent->SetRange(100000);
		lightComponent->SetSpot(4.0f);

		// Add light
		auto light2 = SceneEntity::Create("Light2");

		//light->AddComponent<CTransform>(vec3(87000, 5000, 67000));
		light2->AddComponent<CTransform>(vec3(0, 0.5, 0));

		lightComponent = light2->AddComponent<CLight>();
		lightComponent->SetMaterial(vec4(1, 1, 1, 1));
		lightComponent->SetDirection(vec3(-1, -1, -1));
		lightComponent->SetAtt(1, 1, 0);
		lightComponent->SetIntensity(0.3f, 1.0f, 1.0f);
		lightComponent->SetType(Vulkan::LightType::DIRECTIONAL_LIGHT);
		lightComponent->SetRange(100000);
		lightComponent->SetSpot(4.0f);

		World::Instance().Update();

		SceneRenderer::Instance().InitShader();
		ObjectManager::Instance().PrintObjects();

		/*CMesh* mesh = entity->AddComponent<CMesh>("data/models/teapot.obj");
		mesh->SetPipeline(PipelineType::PIPELINE_BASIC);

		CLight* light = entity->AddComponent<CLight>();
		CParticleSystem* particleSystem = entity->AddComponent<CParticleSystem>();

		Entity* cameraEntity = Entity::Create("Camera");
		CCamera* camera = cameraEntity->AddComponent<CCamera>();
		camera->SetPosition(vec3(0, 0, 0));*/
	}

	void Game::Update()
	{
		World::Instance().Update();
		SceneRenderer::Instance().Update();
		mRenderer->Update();
	}

	void Game::Draw()
	{
		SceneRenderer::Instance().Render();
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
			mTimer.FrameBegin();

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
				auto fps = mTimer.FrameEnd();

				// Only display fps when 1.0s have passed
				if (fps != -1)
				{
					std::stringstream ss;
					ss << "Utopian Engine (alpha) ";
					ss << "FPS: " << mTimer.GetFPS();
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