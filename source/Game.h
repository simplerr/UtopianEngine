#pragma once

#include "Platform.h"
#include "Timer.h"

namespace ECS
{
	class SystemManager;
	class Entity;
}

class Terrain;
class Input;
class SceneRenderer;

namespace Vulkan
{
	class Renderer;
	class Window;
	class Camera;
	class ModelLoader;

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
		void InitTestScene();	
		bool IsClosing();

		// Move all of these to other locations
		Renderer* mRenderer;
		Window* mWindow;
		Camera* mCamera;
		Terrain* mTerrain;
		SceneRenderer*  mSceneRenderer;
		Timer mTimer;
		std::string mTestCaseName;
		bool mIsClosing;

		ECS::SystemManager* mEntityManager;
		ECS::Entity* mTestEntity;
		Input* mInput;
	};
}
