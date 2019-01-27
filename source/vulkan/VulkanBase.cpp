#include <vector>
#include <array>
#include <cassert>
#include <sstream>
#include <chrono>
#include "utility/Platform.h"
#include "VulkanBase.h"
#include "Device.h"
#include "VulkanDebug.h"
#include "Window.h"
#include "ShaderFactory.h"
#include "handles/CommandPool.h"
#include "handles/Semaphore.h"
#include "handles/Image.h"
#include "handles/RenderPass.h"
#include "handles/Instance.h"
#include "handles/FrameBuffers.h"
#include "handles/Queue.h"

namespace Utopian::Vk
{
	VulkanBase::VulkanBase(bool enableValidation)
	{
		VulkanDebug::SetupDebugLayers();

		// Create VkInstance
		mInstance = new Instance("Utopian Engine (pre-alpha)", enableValidation);

		VulkanDebug::InitDebug(mInstance->GetVkHandle());

		mDevice = new Device(mInstance);

		// Setup function pointers for the swap chain
		mSwapChain.connect(mInstance->GetVkHandle(), mDevice->GetPhysicalDevice(), mDevice->GetVkDevice());
	}

	VulkanBase::~VulkanBase()
	{
		mSwapChain.cleanup();

		delete mDepthStencil;
		delete mRenderPass;
		delete mFrameBuffers;
		delete mDevice;

		VulkanDebug::CleanupDebugging(mInstance->GetVkHandle());

		delete mInstance;
	}

	void VulkanBase::Prepare()
	{
		CompileShaders();
		SetupSwapchain();

		mDepthStencil = new Image(mDevice, GetWindowWidth(), GetWindowHeight(),
			mDepthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		
		mRenderPass = new RenderPass(mDevice, mColorFormat, mDepthFormat, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		mRenderPass->Create();
		mFrameBuffers = new FrameBuffers(mDevice, mRenderPass, mDepthStencil, &mSwapChain, GetWindowWidth(), GetWindowHeight());
	}

	void VulkanBase::SetupSwapchain()
	{
		uint32_t width = GetWindowWidth();
		uint32_t height = GetWindowHeight();
		mSwapChain.create(&width, &height);
	}

	void VulkanBase::PrepareFrame()
	{
		Queue* queue = mDevice->GetQueue();
		VulkanDebug::ErrorCheck(mSwapChain.acquireNextImage(queue->GetWaitSemaphore()->GetVkHandle(),
													        &mFrameBuffers->mCurrentFrameBuffer));
	}

	void VulkanBase::SubmitFrame()
	{
		Queue* queue = mDevice->GetQueue();
		VulkanDebug::ErrorCheck(mSwapChain.queuePresent(queue->GetVkHandle(),
														mFrameBuffers->mCurrentFrameBuffer,
														queue->GetSignalSemaphore()->GetVkHandle()));

		queue->WaitIdle();
	}

	void VulkanBase::InitSwapchain(Utopian::Window* window)
	{
		mWindow = window;

#if defined(_WIN32)
		mSwapChain.initSurface(mWindow->GetInstance(), mWindow->GetHwnd());
#elif defined(__linux__)
		mSwapChain.initSurface(mWindow->GetConnection(), mWindow->GetWindow());
#endif
	}

	Device* VulkanBase::GetDevice()
	{
		return mDevice;
	}

	VkDevice VulkanBase::GetVkDevice()
	{
		return mDevice->GetVkDevice();
	}

	RenderPass* VulkanBase::GetRenderPass()
	{
		return mRenderPass;
	}

	void VulkanBase::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Game::HandleMessages() handles the closing of the window
	}

	int VulkanBase::GetWindowWidth()
	{
		return mWindow->GetWidth();
	}

	int VulkanBase::GetWindowHeight()
	{
		return mWindow->GetHeight();
	}

	VkFramebuffer VulkanBase::GetCurrentFrameBuffer()
	{
		return mFrameBuffers->GetCurrent();
	}

	Window* VulkanBase::GetWindow()
	{
		return mWindow;
	}
	
	VkFormat VulkanBase::GetColorFormat()
	{
		return mColorFormat;	
	}
	
	VkFormat VulkanBase::GetDepthFormat()
	{
		return mDepthFormat;	
	}
}	// VulkanLib namespace