#include <string>
#include <time.h>
#include "core/Input.h"
#include "core/Log.h"
#include "core/renderer/Renderer.h"
#include "core/Window.h"
#include "core/LuaManager.h"
#include "core/Profiler.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/RendererUtility.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "vulkan/Debug.h"
#include "vulkan/handles/Device.h"
#include "vulkan/EffectManager.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/ShaderFactory.h"
#include "RayTrace.h"

RayTrace::RayTrace(Utopian::Window* window)
	: mWindow(window)
{
	srand(time(NULL));

	Utopian::Vk::Debug::TogglePerformanceWarnings();
	Utopian::Vk::Debug::SetupDebugLayers();

	gLog().Start();

	mVulkanApp = std::make_shared<Utopian::Vk::VulkanApp>(window);
	mVulkanApp->Prepare();

	// Load modules
	Vk::Device* device = mVulkanApp->GetDevice();
	Vk::gEffectManager().Start();
	Vk::gTextureLoader().Start(device);
	Vk::gModelLoader().Start(device);
	Vk::gShaderFactory().Start(device);
	Vk::gShaderFactory().AddIncludeDirectory("data/shaders/include");

	gInput().Start();
	gTimer().Start();
	gLuaManager().Start();
	gProfiler().Start();
	gRendererUtility().Start();

	mImGuiRenderer = std::make_shared<ImGuiRenderer>(mVulkanApp.get(), mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());

	InitScene();
}

RayTrace::~RayTrace()
{
	Vk::gShaderFactory().Destroy();
	Vk::gEffectManager().Destroy();
	Vk::gTextureLoader().Destroy();
	Vk::gModelLoader().Destroy();

	gTimer().Destroy();
	gInput().Destroy();
	gLuaManager().Destroy();
	gProfiler().Destroy();
	gRendererUtility().Destroy();
}

void RayTrace::Update()
{
	mImGuiRenderer->NewFrame();

	ImGuiRenderer::BeginWindow("Raytracing demo", glm::vec2(10, 150), 300.0f);

	static bool shadows = false, normals = false, ssao = false;
	ImGui::Checkbox("Shadows", &shadows);
	ImGui::Checkbox("Normal mapping", &normals);
	ImGui::Checkbox("SSAO", &ssao);
	ImGui::Text("X: %f, Y: %f", gInput().GetMousePosition().x, gInput().GetMousePosition().y);

	ImGuiRenderer::EndWindow();

	mImGuiRenderer->EndFrame();
}

void RayTrace::Draw()
{
	if (mVulkanApp->PreviousFrameComplete())
	{
		gTimer().FrameEnd();

		mVulkanApp->PrepareFrame();

		mImGuiRenderer->Render();

		// Present to screen
		mVulkanApp->Render();

		mVulkanApp->SubmitFrame();
		gTimer().FrameBegin();
	}
}

void RayTrace::Run()
{
	while (true)
		{
			bool closeWindow = mWindow->DispatchMessages();

			if (!closeWindow)
			{
				Update();
				Draw();
				Utopian::gInput().Update(0);
			}
			else
			{
				break;
			}
		}
}

void RayTrace::InitScene()
{
	
}

void RayTrace::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(mWindow->GetHwnd());
		PostQuitMessage(0);
		break;
	}

	mVulkanApp->HandleMessages(hWnd, uMsg, wParam, lParam);
	gInput().HandleMessages(uMsg, wParam, lParam);
}