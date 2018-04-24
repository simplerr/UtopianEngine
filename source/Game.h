#pragma once

#include "vulkan/VulkanInclude.h"
#include "utility/Platform.h"
#include "utility/Timer.h"
#include "utility/Common.h"

class Terrain;
class Input;

namespace Utopian
{
	class RenderingManager;
	class Editor;
}

namespace Utopian
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
		SharedPtr<Vk::Renderer> mRenderer;
		SharedPtr<Camera> mCamera;
		SharedPtr<Terrain> mTerrain;
		SharedPtr<Editor> mEditor;
		Window* mWindow;
		Timer mTimer;
		std::string mTestCaseName;
		bool mIsClosing;
	};
}
