#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "VulkanApp.h"
#include "Object.h"
#include <string>
#include <sstream>
#include <fstream>
#include "ModelLoader.h"
#include "EntityManager.h"
#include "Entity.h"
#include "MeshComponent.h"
#include "TransformComponent.h"

namespace VulkanLib
{
	Game::Game(Window* window)
	{
		mIsClosing = false;
		mRenderer = new VulkanApp();

		mRenderer->InitSwapchain(window);
		mRenderer->Prepare();

		mWindow = window;

		// Create the camera
		mCamera = new VulkanLib::Camera(glm::vec3(500, 4700, 500), 60.0f, (float)mWindow->GetWidth() / (float)mWindow->GetHeight(), 0.1f, 25600.0f);
		mCamera->LookAt(glm::vec3(0, 0, 0));
		mRenderer->SetCamera(mCamera);

		mEntityManager = new ECS::EntityManager(mRenderer);

		mModelLoader = new ModelLoader();

		InitScene();
	}

	Game::~Game()
	{
		mModelLoader->CleanupModels(mRenderer->GetDevice());
		delete mModelLoader;

		delete mCamera;
		delete mEntityManager;
		delete mRenderer;
	}

	void Game::InitScene()
	{
		// Create example Entity
		ECS::ComponentList componentList;
		componentList.push_back(new ECS::MeshComponent("data/models/teapot.obj", ECS::Pipeline::PIPELINE_BASIC));
		componentList.push_back(new ECS::TransformComponent(vec3(0, -100, 0)));
		
		ECS::Entity* testEntity = mEntityManager->AddEntity(componentList);

		//TransformComponent transform = mEntityManager->GetComponent<TransformComponent>(testEntity);

		//mEntityManager->SendQuery()

		// Add a test object to the scene
		// Add objects
		int size = 5;
		int i = 0;
		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				for (int z = 0; z < size; z++)
				{
					int space = 300;
					Object* object = new Object(glm::vec3(x * space, -100 - y * space, z * space));
					object->SetModel("data/models/teapot.obj");
					object->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
					object->SetId(OBJECT_ID_PROP);
					object->SetRotation(glm::vec3(180, 0, 0));
					object->SetScale(glm::vec3(3.0f));
					object->SetPipeline(PipelineEnum::COLORED);

					VulkanModel model;
					model.object = object;
					model.mesh = mModelLoader->LoadModel(mRenderer->GetDeviceTmp(), object->GetModel());
					mRenderer->AddModel(model);
				}
			}
		}
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
				mRenderer->Update();
				mRenderer->Render();
				mEntityManager->Process();

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