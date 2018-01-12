#pragma once

#include "vulkan/VulkanInclude.h"
#include "Platform.h"
#include "Timer.h"
#include "utility/Common.h"

class Terrain;
class Input;

namespace Scene
{
	class SceneRenderer;
}

namespace Vulkan
{
	class Game
	{
	public:
		Game(Window* window);
		~Game();

		void RenderLoop();
		void Update();
		void Draw();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		void InitScene();	
		bool IsClosing();

		// Move all of these to other locations
		SharedPtr<Renderer> mRenderer;
		SharedPtr<Camera> mCamera;
		SharedPtr<Terrain> mTerrain;
		Window* mWindow;
		Timer mTimer;
		std::string mTestCaseName;
		bool mIsClosing;
	};
}
