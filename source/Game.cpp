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
#include "core/renderer/RenderingManager.h"
#include "editor/Editor.h"
#include "utility/Utility.h"

using namespace Utopian;

namespace Utopian
{
	Game::Game(Window* window) 
		: mWindow(window)
	{
		srand(time(NULL));

		mIsClosing = false;

		Utopian::Vk::VulkanDebug::TogglePerformanceWarnings();

		mRenderer = make_shared<Vk::Renderer>();
		mRenderer->InitSwapchain(window);
		mRenderer->Prepare();
		mRenderer->SetClearColor(ColorRGB(47, 141, 255));

		mTerrain = make_shared<Terrain>(mRenderer.get());
		mTerrain->SetEnabled(false);

		InitScene();

		mRenderer->PostInitPrepare();

		// Note: Needs to be called after a camera have been added to the scene
		mEditor = make_shared<Editor>(mRenderer.get(), &World::Instance(), mTerrain.get());


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
		Vk::ShaderFactory::Start(mRenderer->GetDevice());
		Vk::ShaderFactory::Instance().AddIncludeDirectory("data/shaders/include");

		RenderingManager::Start(mRenderer.get());
		RenderingManager::Instance().SetTerrain(mTerrain.get());


		//// Add house
		//auto house = Actor::Create("House_1");

		//CTransform* transform = house->AddComponent<CTransform>(vec3(61000.0f, 300.0f, 78000.0f));
		//transform->SetScale(vec3(550.0f));
		//transform->SetRotation(vec3(180, 0, 0));

		//CRenderable* mesh = house->AddComponent<CRenderable>();
		//mesh->SetModel(mRenderer->mModelLoader->LoadModel(mRenderer->GetDevice(), "data/models/adventure_village/HouseBricksLarge.obj"));
		//
		//// Add camera
		auto cameraEntity = Actor::Create("Camera");
		cameraEntity->AddComponent<CTransform>(vec3(67001.0f, 10300.0f, 68000.0f));
		CCamera* camera = cameraEntity->AddComponent<CCamera>(mWindow, 60.0f, 10.0f, 256000.0f);
		camera->SetMainCamera();

		cameraEntity->AddComponent<CNoClip>(300.0f);
		COrbit* orbit = cameraEntity->AddComponent<COrbit>(0.01f);
		orbit->SetTarget(vec3(81000.0f, 5300.0f, 78000.0f));

		cameraEntity->AddComponent<CPlayerControl>();

		//// Add house
		//house = Actor::Create("Well_1");

		//transform = house->AddComponent<CTransform>(vec3(91000.0f, 6300.0f, 78000.0f));
		//transform->SetScale(vec3(1550.0f));
		//transform->SetRotation(vec3(0, 0, 0));

		//mesh = house->AddComponent<CRenderable>();
		//mesh->SetModel(mRenderer->mModelLoader->LoadModel(mRenderer->GetDevice(), "data/models/arrow.obj"));

		// Add actor w/ directional light component

		/************************************************************************/
		/* Castle
		/************************************************************************/
		SharedPtr<Actor> castle = Actor::Create("Castle");

		CTransform* transform2 = castle->AddComponent<CTransform>(vec3(94400.0f, 6200.0f, 78000.0f));
		transform2->SetScale(vec3(50.0f));
		transform2->SetRotation(vec3(180, 0, 0));

		CRenderable* mesh2 = castle->AddComponent<CRenderable>();
		mesh2->SetModel(mRenderer->mModelLoader->LoadModel(mRenderer->GetDevice(), "data/models/sponza/sponza.obj"));
		//mesh->SetModel(mRenderer->mModelLoader->LoadModel(mRenderer->GetDevice(), "data/models/sponza_lowres/sponza.obj"));
		//mesh->SetMaterial(Vk::Mat(Vk::EffectType::COLOR, Vk::ColorEffect::NORMAL));

		CLight* lightComponent2 = castle->AddComponent<CLight>();
		lightComponent2->SetMaterial(vec4(1, 1, 1, 1));
		lightComponent2->SetDirection(vec3(1, 0, 0));
		lightComponent2->SetAtt(0, 0.00, 0.00000002);
		lightComponent2->SetIntensity(0.3f, 0.5f, 0.0f);
		lightComponent2->SetType(Utopian::Vk::LightType::DIRECTIONAL_LIGHT);
		lightComponent2->SetRange(100000);
		lightComponent2->SetSpot(4.0f);

		/************************************************************************/
		/* Teapot
		/************************************************************************/
		SharedPtr<Actor> teapot = Actor::Create("Teapot");

		CTransform* transform1 = teapot->AddComponent<CTransform>(vec3(74400.0f, 8200.0f, 78000.0f));
		transform1->SetScale(vec3(50.0f));
		transform1->SetRotation(vec3(0, 0, 0));

		CRenderable* mesh1 = teapot->AddComponent<CRenderable>();
		mesh1->SetModel(mRenderer->mModelLoader->LoadModel(mRenderer->GetDevice(), "data/models/teapot.obj"));
		mesh1->SetColor(glm::vec4(0.3, 1, 0.2, 1.0));

		CLight* lightComponent1 = teapot->AddComponent<CLight>();
		lightComponent1->SetMaterial(vec4(1, 1, 1, 1));
		lightComponent1->SetDirection(vec3(1, 0, 0));
		lightComponent1->SetAtt(0, 0.00, 0.00000002);
		lightComponent1->SetIntensity(0.3f, 1.0f, 0.0f);
		lightComponent1->SetType(Utopian::Vk::LightType::POINT_LIGHT);
		lightComponent1->SetRange(100000);
		lightComponent1->SetSpot(4.0f);

		// Add orbiting entity
		//auto teapot = Actor::Create("Window");

		//transform = teapot->AddComponent<CTransform>(vec3(81000.0f, 5300.0f, 78000.0f));
		//transform->SetScale(vec3(1050.0f));
		//transform->SetRotation(vec3(0, 180, 180));

		///*orbit = teapot->AddComponent<COrbit>(-0.01f);
		//orbit->SetTarget(vec3(81000.0f, 5300.0f, 78000.0f));*/

		//mesh = teapot->AddComponent<CRenderable>();
		//mesh->SetModel(mRenderer->mModelLoader->LoadModel(mRenderer->GetDevice(), "data/models/adventure_village/HouseTower.obj"));

		//// Add light
		//auto light2 = Actor::Create("PointLight");

		//light2->AddComponent<CTransform>(vec3(-74400.0f, -6200.0f, -78000.0f));

		//lightComponent = light2->AddComponent<CLight>();
		//lightComponent->SetMaterial(vec4(1, 0, 0, 1));
		//lightComponent->SetDirection(vec3(0, -1, 0));
		//lightComponent->SetAtt(0, 0.00, 0.00000002);
		//lightComponent->SetIntensity(0.0, 1.0f, 0.0f);
		//lightComponent->SetType(Utopian::Vk::LightType::POINT_LIGHT);
		//lightComponent->SetRange(400000);
		//lightComponent->SetSpot(4.0f);

		//// Add spot light
		//auto spotLight = Actor::Create("SpotLight");

		//spotLight->AddComponent<CTransform>(vec3(-104400.0f, -6200.0f, -78000.0f));

		//lightComponent = spotLight->AddComponent<CLight>();
		//lightComponent->SetMaterial(vec4(0, 0, 1, 1));
		//lightComponent->SetDirection(vec3(1, 1, 1));
		//lightComponent->SetAtt(0, 0.00, 0.000000002);
		//lightComponent->SetIntensity(0.0, 1.0f, 0.0f);
		//lightComponent->SetType(Utopian::Vk::LightType::SPOT_LIGHT);
		//lightComponent->SetRange(400000);
		//lightComponent->SetSpot(8.0f);

		World::Instance().Update();

		RenderingManager::Instance().InitShaderResources();
		RenderingManager::Instance().InitShader();
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
		mRenderer->BeginUiUpdate();

		World::Instance().Update();
		RenderingManager::Instance().Update();
		mEditor->Update();
		mRenderer->Update();

		mRenderer->EndUiUpdate();
	}

	void Game::Draw()
	{
		RenderingManager::Instance().Render();
		mEditor->Draw();
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