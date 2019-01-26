#pragma once

#if defined(_WIN32)
#pragma comment(linker, "/subsystem:windows")
#endif

#include "vulkan/vulkanswapchain.hpp"
#include "vulkan/VulkanInclude.h"
#include "Window.h"
#include "utility/Timer.h"

/*
Resources

Validation layer guide: http://gpuopen.com/using-the-vulkan-validation-layers/
Vulkan specification: https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html
Vulkan spec + WSI: https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html
Vulkan in 30 minutes: https://renderdoc.org/vulkan-in-30-minutes.html
Memory management: https://developer.nvidia.com/vulkan-memory-management
Niko Kauppi videoes: https://www.youtube.com/watch?v=Bu581jeyTL0
Pipeleline barriers: https://github.com/philiptaylor/vulkan-sxs/tree/master/04-clear
Intel Vulkan tutorial: https://software.intel.com/en-us/api-without-secrets-introduction-to-vulkan-part-2
Shader resource bindings: https://developer.nvidia.com/vulkan-shader-resource-binding
Installation on Ubuntu: http://www.trentreed.net/blog/installing-nvidia-vulkan-driver-and-lunarg-sdk-on-ubuntu/

Vulkan Tutorial: https://vulkan-tutorial.com/
*/


namespace Utopian::Vk
{
	// This is the base class that contains common code for creating a Vulkan application
	class VulkanBase
	{
	public:
		VulkanBase(bool enableValidation);
		virtual ~VulkanBase();

		virtual void Prepare();

		virtual void Render() = 0;

		void InitSwapchain(Utopian::Window* window);
		void SetupSwapchain();

		void PrepareFrame();
		void SubmitFrame();

		virtual void CompileShaders() = 0;

		Device* GetDevice();
		VkDevice GetVkDevice();
		RenderPass* GetRenderPass();
		VkFramebuffer GetCurrentFrameBuffer();
		VkFormat GetColorFormat();
		VkFormat GetDepthFormat();

		int GetWindowWidth();
		int GetWindowHeight();


		Window*  GetWindow();

		// Platform specific
#if defined(_WIN32)
		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
		// TODO: TEMP: should be protected
		FrameBuffers*					mFrameBuffers = nullptr;

	protected:
		// Swap chain magic by Sascha Willems (https://github.com/SaschaWillems/Vulkan)
		VulkanSwapChain					mSwapChain;

		Instance*						mInstance = nullptr;
		Device*							mDevice = nullptr;
		Window*							mWindow = nullptr;
		RenderPass*						mRenderPass = nullptr;
		Image*							mDepthStencil = nullptr;

		VkFormat						mColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		VkFormat						mDepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
	};
}	// VulkanLib namespace
#pragma once
