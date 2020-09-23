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

	RayTrace(Utopian::Window* window);
	~RayTrace();

	void Run();

	void Update();
	void Draw();

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitScene();
	void CalculateRays();

	SharedPtr<Vk::VulkanApp> mVulkanApp = nullptr;
	SharedPtr<ImGuiRenderer> mImGuiRenderer = nullptr;
	Utopian::Window* mWindow;

	// Test
	SharedPtr<Vk::Effect> mEffect;
	SharedPtr<Vk::Semaphore> mRayTraceComplete;
	SharedPtr<Vk::Image> mOutputImage;
	SharedPtr<Vk::Sampler> mSampler;


	InputParameters mInputParameters;
	glm::vec3 mCameraPos;
	glm::vec3 mCameraTarget;
};
