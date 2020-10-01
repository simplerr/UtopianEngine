#include "MarchingCubes.h"
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
#include "core/MiniCamera.h"
#include "core/Engine.h"

MarchingCubes::MarchingCubes(Utopian::Window* window)
	: mWindow(window)
{
	// Start Utopian Engine
	Utopian::gEngine().Start(window, "Marching Cubes demo");
	Utopian::gEngine().StartModules();
	Utopian::gEngine().RegisterUpdateCallback(&MarchingCubes::UpdateCallback, this);
	Utopian::gEngine().RegisterRenderCallback(&MarchingCubes::DrawCallback, this);
	Utopian::gEngine().RegisterDestroyCallback(&MarchingCubes::DestroyCallback, this);

	mVulkanApp = Utopian::gEngine().GetVulkanApp();

	mRayTraceComplete = std::make_shared<Vk::Semaphore>(mVulkanApp->GetDevice());
	//mVulkanApp->SetWaitSubmitSemaphore(mRayTraceComplete);

	InitResources();
}

MarchingCubes::~MarchingCubes()
{
	Utopian::gEngine().Destroy();
}

void MarchingCubes::DestroyCallback()
{
	// Free Vulkan resources
	mEffect = nullptr;
	mRayTraceComplete = nullptr;
	mOutputImage = nullptr;
	mSampler = nullptr;

	mInputParameters.GetBuffer()->Destroy();
}

void MarchingCubes::InitResources()
{
	uint32_t width = mWindow->GetWidth();
	uint32_t height = mWindow->GetHeight();
	Vk::Device* device = mVulkanApp->GetDevice();

	mCamera = std::make_shared<MiniCamera>(glm::vec3(5, 25, 5), glm::vec3(25, 0, 25), 1.0f, width, height);

	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/marching_cubes_demo/marching_cubes.comp";
	mEffect = Vk::Effect::Create(device, nullptr, effectDesc);

	mInputParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	mOutputImage = std::make_shared<Vk::ImageStorage>(device, width, height, "Raytrace image");
	mSampler = std::make_shared<Vk::Sampler>(device);

	mEffect->BindImage("outputImage", *mOutputImage);
	mEffect->BindUniformBuffer("UBO_input", mInputParameters);

	gScreenQuadUi().AddQuad(0, 0, width, height, mOutputImage.get(), mSampler.get());
}

void MarchingCubes::UpdateCallback()
{
	// ImGuiRenderer::BeginWindow("Raytracing Demo", glm::vec2(10, 150), 300.0f);
	// ImGui::SliderInt("Max trace depth", &mSettingParameters.data.maxTraceDepth, 1, 8);
	// ImGuiRenderer::EndWindow();

	// Recompile shaders
	if (gInput().KeyPressed('R'))
	{
		Vk::gEffectManager().RecompileModifiedShaders();
	}

	mCamera->Update();
}

void MarchingCubes::DrawCallback()
{
	// Update uniforms
	mInputParameters.data.time = 0.0f;
	mInputParameters.UpdateMemory();

	// Test rendering
	Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	commandBuffer.Begin();
	commandBuffer.CmdBindPipeline(mEffect->GetPipeline());
	commandBuffer.CmdBindDescriptorSets(mEffect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
	commandBuffer.CmdDispatch(mWindow->GetWidth() / 16, mWindow->GetHeight() / 16, 1);
	commandBuffer.Flush();

	// Todo: Should be in Engine somewhere
	gScreenQuadUi().Render(mVulkanApp);
}

void MarchingCubes::Run()
{
	Utopian::gEngine().Run();
}

void MarchingCubes::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Utopian::gEngine().HandleMessages(hWnd, uMsg, wParam, lParam);
}