#pragma once

#if defined(_WIN32)
#pragma comment(linker, "/subsystem:windows")
#endif

#include <vulkan/vulkan.h>
#include "vulkan/vulkanswapchain.hpp"
#include "Window.h"
#include "Timer.h"

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


namespace VulkanLib
{
	class Device;
	class CommandPool;
	class Semaphore;
	class Image;
	class RenderPass;
	class Instance;
	class FrameBuffers;
	class ShaderManager;
	class Queue;

	// This is the base class that contains common code for creating a Vulkan application
	class VulkanBase
	{
	public:
		VulkanBase(bool enableValidation);
		virtual ~VulkanBase();

		virtual void Prepare();

		virtual void Update() = 0;
		virtual void Render() = 0;

		void InitSwapchain(Window* window);
		void SetupSwapchain();

		// [TODO] Remove
		VkBool32 CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory);

		void PrepareFrame();
		void SubmitFrame();

		virtual void CompileShaders() = 0;

		VkDevice GetDevice();
		CommandPool* GetCommandPool();

		int GetWindowWidth();
		int GetWindowHeight();

		// Platform specific
#if defined(_WIN32)
		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

	protected:
		// Swap chain magic by Sascha Willems (https://github.com/SaschaWillems/Vulkan)
		VulkanSwapChain					mSwapChain;

		Instance*						mInstance = nullptr;
		Device*							mDevice = nullptr;
		FrameBuffers*					mFrameBuffers = nullptr;
		Queue*							mQueue = nullptr;
		CommandPool*					mCommandPool = nullptr;
		Window*							mWindow = nullptr;
		RenderPass*						mRenderPass = nullptr;
		Semaphore*						mPresentComplete = nullptr;
		Semaphore*						mRenderComplete = nullptr;
		Image*							mDepthStencil = nullptr;
		ShaderManager*					mShaderManager = nullptr;

		VkFormat						mDepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		VkFormat						mColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	};
}	// VulkanLib namespace
#pragma once
