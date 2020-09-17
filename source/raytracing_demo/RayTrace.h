#pragma once

#include <string>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VulkanApp.h"
#include "utility/Common.h"

using namespace Utopian;

class RayTrace
{
public:
	RayTrace(Utopian::Window* window);
	~RayTrace();

	void Run();

	void Update();
	void Draw();

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitScene();

	SharedPtr<Vk::VulkanApp> mVulkanApp = nullptr;
	SharedPtr<ImGuiRenderer> mImGuiRenderer = nullptr;
	Utopian::Window* mWindow;
};
