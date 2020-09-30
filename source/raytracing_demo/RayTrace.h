#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vulkan/RenderTarget.h>
#include <vulkan/handles/Semaphore.h>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/ShaderBuffer.h"
#include "utility/Common.h"

using namespace Utopian;

class MiniCamera;

class RayTrace
{
public:
	UNIFORM_BLOCK_BEGIN(InputParameters)
		UNIFORM_PARAM(glm::vec4, eye)
		UNIFORM_PARAM(glm::vec4, ray00)
		UNIFORM_PARAM(glm::vec4, ray01)
		UNIFORM_PARAM(glm::vec4, ray10)
		UNIFORM_PARAM(glm::vec4, ray11)
	UNIFORM_BLOCK_END()

	UNIFORM_BLOCK_BEGIN(SettingsParameters)
		UNIFORM_PARAM(int, maxTraceDepth)
	UNIFORM_BLOCK_END()

	RayTrace(Utopian::Window* window);
	~RayTrace();

	void Run();

	void DestroyCallback();
	void UpdateCallback();
	void DrawCallback();

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitScene();
	void CalculateRays();

	Vk::VulkanApp* mVulkanApp;
	Utopian::Window* mWindow;

	// Test
	SharedPtr<Vk::Effect> mEffect;
	SharedPtr<Vk::Semaphore> mRayTraceComplete;
	SharedPtr<Vk::Image> mOutputImage;
	SharedPtr<Vk::Sampler> mSampler;

	SharedPtr<MiniCamera> mCamera;
	InputParameters mInputParameters;
	SettingsParameters mSettingParameters;
};
