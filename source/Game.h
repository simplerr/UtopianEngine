#pragma once
#include "Platform.h"
#include "Timer.h"

namespace ECS
{
	class EntityManager;
}

namespace VulkanLib
{
	class VulkanApp;
	class Window;
	class Camera;
	class ModelLoader;

	class Game
	{
	public:
		Game(Window* window);
		~Game();

		void RenderLoop();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		void InitScene();	
		bool IsClosing();

		// Move all of these to other locations
		VulkanApp* mRenderer;
		Window* mWindow;
		Camera* mCamera;
		ModelLoader* mModelLoader;
		Timer mTimer;
		std::string mTestCaseName;
		bool mIsClosing;

		ECS::EntityManager* mEntityManager;
	};
}
