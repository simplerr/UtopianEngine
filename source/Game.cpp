#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include <string>
#include <sstream>
#include <fstream>

namespace VulkanLib
{
	Game::Game(Window* window)
	{
		mRenderer = nullptr;
		mWindow = window;

		// Create the camera
		mCamera = new VulkanLib::Camera(glm::vec3(500, 4700, 500), 60.0f, (float)mWindow->GetWidth() / (float)mWindow->GetHeight(), 0.1f, 25600.0f);
		mCamera->LookAt(glm::vec3(0, 0, 0));
	}

	Game::~Game()
	{
		delete mRenderer;
		delete mCamera;
	}

	void Game::InitScene()
	{
		mRenderer->SetCamera(mCamera);

		// Creates the instancing array from all the objects
		mRenderer->Init();
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

			if (mRenderer != nullptr)
			{
				mRenderer->Update();
				mRenderer->Render();

				// Frame end
				auto fps = mTimer.FrameEnd();

				// Only display fps when 1.0s have passed
				if (fps != -1)
				{
					std::stringstream ss;
					ss << "Project Vulkan: " << mRenderer->GetName() << " [" << fps << " fps] [" << mRenderer->GetNumVertices() << " vertices] [" << mRenderer->GetNumTriangles() << " triangles] [" << mRenderer->GetNumObjects() << " objects]";
					ss << " [" << mRenderer->GetNumThreads() << " threads]";
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
			break;
		}
	}
}