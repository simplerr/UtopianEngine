#pragma once

#include <string>
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian
{
	class Editor;
}

class Game
{
public:
	Game(Utopian::Window* window);
	~Game();

	void Run();

	void DestroyCallback();
	void UpdateCallback();
	void DrawCallback();

	virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitScene();
	void AddGround();
	void AddBoxes();

	// Move all of these to other locations
	SharedPtr<Utopian::Editor> mEditor;
	Utopian::Window* mWindow;
};
