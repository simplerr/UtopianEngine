#include <string>
#include <time.h>
#include <vulkan/vulkan_core.h>
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
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/CommandBuffer.h"
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

	mRayTraceComplete = std::make_shared<Vk::Semaphore>(mVulkanApp->GetDevice());
	//mVulkanApp->SetWaitSubmitSemaphore(mRayTraceComplete);

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
	gProfiler().Start(mVulkanApp.get());
	gRendererUtility().Start();
	gScreenQuadUi().Start(mVulkanApp.get());

	mImGuiRenderer = std::make_shared<ImGuiRenderer>(mVulkanApp.get(), mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());

	InitScene();
}

RayTrace::~RayTrace()
{
	// Vulkan handles cannot be destroyed when they are in use on the GPU
	while (!mVulkanApp->PreviousFrameComplete())
	{
	}

	Vk::gShaderFactory().Destroy();
	Vk::gEffectManager().Destroy();
	Vk::gTextureLoader().Destroy();
	Vk::gModelLoader().Destroy();

	gTimer().Destroy();
	gInput().Destroy();
	gLuaManager().Destroy();
	gScreenQuadUi().Destroy();
	gProfiler().Destroy();
	gRendererUtility().Destroy();
}

void RayTrace::InitScene()
{
	Vk::IMAGE_CREATE_INFO createInfo;
	createInfo.width = mWindow->GetWidth();
	createInfo.height = mWindow->GetHeight();
	createInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createInfo.finalImageLayout = VK_IMAGE_LAYOUT_GENERAL;
	createInfo.name = "Raytrace image";
	mOutputImage = std::make_shared<Vk::Image>(createInfo, mVulkanApp->GetDevice());

	// Transition to correct layout
	Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	mOutputImage->LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_GENERAL);
	commandBuffer.Flush();

	mSampler = std::make_shared<Vk::Sampler>(mVulkanApp->GetDevice());

	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/raytracing_demo/raytrace.comp";
	mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mVulkanApp->GetDevice(), nullptr, effectDesc);
	mEffect->BindImage("outputImage", *mOutputImage);

	gScreenQuadUi().AddQuad(0, 0, mWindow->GetWidth(), mWindow->GetHeight(), mOutputImage.get(), mSampler.get());
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

	// Recompile shaders
	if (gInput().KeyPressed('R'))
	{
		Vk::gEffectManager().RecompileModifiedShaders();
	}

	gProfiler().Update();

	mImGuiRenderer->EndFrame();
}

void RayTrace::Draw()
{
	if (mVulkanApp->PreviousFrameComplete())
	{
		gTimer().FrameEnd();

		mVulkanApp->PrepareFrame();

		// Test rendering
		Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		commandBuffer.Begin();
		commandBuffer.CmdBindPipeline(mEffect->GetPipeline());
		commandBuffer.CmdBindDescriptorSets(mEffect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
		commandBuffer.CmdDispatch(mWindow->GetWidth() / 16, mWindow->GetHeight() / 16, 1);
		commandBuffer.Flush();

		mImGuiRenderer->Render();
		gScreenQuadUi().Render(mVulkanApp.get());

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