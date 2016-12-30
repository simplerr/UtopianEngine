#pragma once
#include "Platform.h"
#include "Timer.h"

namespace VulkanLib
{
	class VulkanApp;
	class Window;
	class Camera;

	class Game
	{
	public:
		Game(Window* window);
		~Game();

		void RenderLoop();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		void InitScene();	

		VulkanApp* mRenderer;
		Window* mWindow;
		Camera* mCamera;
		Timer mTimer;
		std::string mTestCaseName;
	};
}
