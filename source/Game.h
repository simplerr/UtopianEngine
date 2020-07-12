#pragma once

#include <string>
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Platform.h"
#include "utility/Timer.h"
#include "utility/Common.h"

class Terrain;
class Input;

namespace Utopian
{
	class Renderer;
	class Editor;
}

class Game
{
public:
	Game(Utopian::Window* window);
	~Game();

	void Run();
	void Update();
	void Draw();

	virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitScene();	
	bool IsClosing();

	// Move all of these to other locations
	SharedPtr<Utopian::Vk::VulkanApp> mVulkanApp;
	SharedPtr<Utopian::Editor> mEditor;
	SharedPtr<Terrain> mTerrain;
	Utopian::Window* mWindow;
	const std::string mAppName = "Utopian Engine (v0.1)";
	bool mIsClosing;
};
