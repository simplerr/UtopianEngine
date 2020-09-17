#pragma once

#if defined(_WIN32)
#pragma comment(linker, "/subsystem:windows")
#endif

#include "vulkan/vulkanswapchain.hpp"
#include "vulkan/VulkanPrerequisites.h"
//#include "vulkan/handles/Semaphore.h"
#include "core/Window.h"
#include "utility/Timer.h"
#include "utility/Common.h"

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
	/** Base class that contains common code for creating a Vulkan application. */
	class VulkanBase
	{
	public:
		VulkanBase(Utopian::Window* window, bool enableValidation);
		virtual ~VulkanBase();

		virtual void Prepare();
		virtual void Render() = 0;

		/** To be called at the start of a frame. */
		void PrepareFrame();

		/** To be called at the end of a frame. */
		void SubmitFrame();

		Device* GetDevice();
		RenderPass* GetRenderPass();
		VkFramebuffer GetCurrentFrameBuffer();
		VkFormat GetColorFormat();
		VkFormat GetDepthFormat();

		uint32_t GetWindowWidth();
		uint32_t GetWindowHeight();

		/** Returns the semaphore that is signaled when a new swapchain image is ready for use. */
		const SharedPtr<Semaphore>& GetImageAvailableSemaphore() const;

		/** Returns the semaphore that is signaled when the submitted primary command buffer is executed, i.e 
		 * the image can be presented. */
		const SharedPtr<Semaphore>& GetRenderCompleteSemaphore() const;

		/** Returns the semaphore which the primary command buffer submission is waiting on. */
		const SharedPtr<Semaphore>& GetWaitSubmitSemaphore() const;

		/** Sets the semaphore which the primary command buffer submission will wait on. This can
		 * be GetImageAvailableSemaphore() directly or a semaphore signaled by another primary command buffer. */
		void SetWaitSubmitSemaphore(const SharedPtr<Semaphore>& waitSemaphore);

		bool PreviousFrameComplete();

		Window* GetWindow();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		void SetupSwapchain();

	protected:
		// Swap chain magic by Sascha Willems (https://github.com/SaschaWillems/Vulkan)
		VulkanSwapChain					mSwapChain;
		FrameBuffers*					mFrameBuffers = nullptr;
		Instance*						mInstance = nullptr;
		Device*							mDevice = nullptr;
		Window*							mWindow = nullptr;
		Image*							mDepthStencil = nullptr;
		SharedPtr<Semaphore>			mImageAvailable = nullptr;
		SharedPtr<Semaphore>			mRenderComplete = nullptr;
		SharedPtr<Semaphore>			mWaitSubmitSemaphore = nullptr;
		SharedPtr<Fence>				mWaitFence = nullptr;

		// Note: Todo: Used by legacy effects
		RenderPass*						mRenderPass = nullptr;

		VkFormat						mColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		VkFormat						mDepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
	};
}	// VulkanLib namespace
#pragma once
