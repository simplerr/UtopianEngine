#include <glm/matrix.hpp>
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
	mCameraPos = glm::vec3(5, 25, 5);
	mCameraTarget = glm::vec3(25, 0, 25);

	mSampler = std::make_shared<Vk::Sampler>(mVulkanApp->GetDevice());
	mOutputImage = std::make_shared<Vk::ImageStorage>(mVulkanApp->GetDevice(), mWindow->GetWidth(), mWindow->GetHeight(), "Raytrace image");

	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/raytracing_demo/raytrace.comp";
	mEffect = Vk::Effect::Create(mVulkanApp->GetDevice(), nullptr, effectDesc);

	mInputParameters.Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	mEffect->BindImage("outputImage", *mOutputImage);
	mEffect->BindUniformBuffer("UBO_input", mInputParameters);

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

		// Update uniforms
		CalculateRays();
		mInputParameters.data.eye = glm::vec4(mCameraPos, 1.0f);
		mInputParameters.UpdateMemory();

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

void RayTrace::CalculateRays()
{
	// Camera/view matrix
	glm::mat4 viewMatrix = glm::lookAt(mCameraPos, mCameraTarget, glm::vec3(0, -1, 0));

	// Projection matrix
	glm::mat4 projectionMatrix = glm::perspective(90, mWindow->GetWidth() / mWindow->GetHeight(), 1, 2);

	// Get inverse of view * proj
	glm::mat4 inverseViewProjection = projectionMatrix * viewMatrix;
	inverseViewProjection = glm::inverse(inverseViewProjection);

	glm::vec4 cameraPos = glm::vec4(mCameraPos, 0.0f);
	// Calculate rays
	glm::vec4 ray00 = inverseViewProjection * glm::vec4(-1, -1, 0, 1);
	ray00 = ray00 / ray00.w;
	ray00 -= cameraPos;

	glm::vec4 ray10 = inverseViewProjection * glm::vec4(+1, -1, 0, 1);
	ray10 = ray10 / ray10.w;
	ray10 -= cameraPos;

	glm::vec4 ray01 = inverseViewProjection * glm::vec4(-1, +1, 0, 1);
	ray01 = ray01 / ray01.w;
	ray01 -= cameraPos;

	glm::vec4 ray11 = inverseViewProjection * glm::vec4(+1, +1, 0, 1);
	ray11 = ray11 / ray11.w;
	ray11 -= cameraPos;

	mInputParameters.data.ray00 = ray00;
	mInputParameters.data.ray01 = ray01;
	mInputParameters.data.ray10 = ray10;
	mInputParameters.data.ray11 = ray11;
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