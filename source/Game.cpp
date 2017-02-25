#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "vulkan/VulkanApp.h"
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include "vulkan/ModelLoader.h"
#include "ecs/EntityManager.h"
#include "ecs/Entity.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/PhysicsComponent.h"
#include "ecs/components/HealthComponent.h"

namespace VulkanLib
{
	Game::Game(Window* window)
	{
		srand(time(NULL));

		mIsClosing = false;
		mRenderer = new VulkanApp();

		mRenderer->InitSwapchain(window);
		mRenderer->Prepare();

		mWindow = window;

		// Create the camera
		mCamera = new VulkanLib::Camera(mWindow, glm::vec3(2700.0f, 2700.0f, 2700.0f), 60.0f, 0.1f, 25600.0f);
		mCamera->LookAt(glm::vec3(0, 0, 0));
		mRenderer->SetCamera(mCamera);

		mEntityManager = new ECS::EntityManager(mRenderer);
		mEntityManager->Init(mRenderer);

		InitScene();
	}

	Game::~Game()
	{
		delete mCamera;
		delete mEntityManager;
		delete mRenderer;
	}

	void Game::InitScene()
	{
		int size = 3;
		int space = 300;
		int i = 0;
		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				for (int z = 0; z < size; z++)
				{
					// Transform
					ECS::TransformComponent* transformComponent = new ECS::TransformComponent(vec3(x * space,  y * space, z * space));
					transformComponent->SetRotation(glm::vec3(180, 30, 40));
					transformComponent->SetScale(glm::vec3(1.0f));

					// Physics
					uint32_t maxSpeed = 2;
					uint32_t maxRotation = 100;
					float divider = 90.0f;
					ECS::PhysicsComponent* physicsComponent = new ECS::PhysicsComponent();
					physicsComponent->SetVelocity(glm::vec3(rand() % maxSpeed, rand() % maxSpeed, rand() % maxSpeed));
					physicsComponent->SetRotationSpeed(glm::vec3((rand() % maxRotation) / divider, (rand() % maxRotation) / divider, (rand() % maxRotation) / divider));
					physicsComponent->SetScaleSpeed(glm::vec3(0.0f));

					// Mesh
					ECS::MeshComponent* meshComponent = nullptr;
					if (x == 0) {
						meshComponent = new ECS::MeshComponent("data/models/Crate/Crate1.obj", PipelineType::PIPELINE_BASIC);
						transformComponent->SetScale(glm::vec3(35.0f));
					}
					else { 
						meshComponent = new ECS::MeshComponent("data/models/teapot.obj", PipelineType::PIPELINE_BASIC);
					}

					// Health
					ECS::HealthComponent* healthComponent = new ECS::HealthComponent(100);

					// Create component list
					ECS::ComponentList componentList;
					componentList.push_back(meshComponent);
					componentList.push_back(transformComponent);
					componentList.push_back(healthComponent);
					//componentList.push_back(physicsComponent);
					
					// TEMPORARY
					//if(i < 1)
						mEntityManager->AddEntity(componentList);

					i++;
				}
			}
		}

		mTestEntity = mEntityManager->GetEntity(0u);
	}

	void Game::Update()
	{
		mRenderer->Update();
		mEntityManager->Process();

		ECS::TransformComponent* transform = dynamic_cast<ECS::TransformComponent*>(mTestEntity->GetComponent(ECS::TRANSFORM_COMPONENT));
		float speed = 5.0f;
		//transform->AddRotation(glm::radians(speed), glm::radians(speed), glm::radians(speed));
	}

	void Game::Draw()
	{
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
		mEntityManager->HandleMessages(hWnd, uMsg, wParam, lParam);

		if (mRenderer != nullptr)
			mRenderer->HandleMessages(hWnd, uMsg, wParam, lParam);

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