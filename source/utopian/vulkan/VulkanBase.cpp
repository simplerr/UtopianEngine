#include <vector>
#include <array>
#include <cassert>
#include <sstream>
#include "utility/Platform.h"
#include "VulkanBase.h"
#include "vulkan/handles/Device.h"
#include "Debug.h"
#include "core/Window.h"
#include "ShaderFactory.h"
#include "handles/CommandPool.h"
#include "handles/Semaphore.h"
#include "handles/Image.h"
#include "handles/Fence.h"
#include "handles/RenderPass.h"
#include "handles/Instance.h"
#include "handles/FrameBuffers.h"
#include "handles/Queue.h"

namespace Utopian::Vk
{
	VulkanBase::VulkanBase(Utopian::Window* window, bool enableValidation)
		: mWindow(window)
	{
		mInstance = new Instance("Utopian Engine (v0.2)", enableValidation);
		Debug::InitDebug(mInstance);

		mDevice = new Device(mInstance);
		DebugLabel::Setup(mDevice);
	}

	VulkanBase::~VulkanBase()
	{
		mSwapChain.cleanup();

      	// Needs to be freed before deleting the device
      	mWaitFence = nullptr;
      	mImageAvailable = nullptr;
      	mRenderComplete = nullptr;
      	mWaitSubmitSemaphore = nullptr;

		delete mDepthStencil;
		delete mRenderPass;
		delete mFrameBuffers;
		delete mDevice;

		Debug::CleanupDebugging(mInstance->GetVkHandle());

		delete mInstance;
	}

	void VulkanBase::Prepare()
	{
		SetupSwapchain();

		IMAGE_CREATE_INFO createInfo;
		createInfo.width = GetWindowWidth();
		createInfo.height = GetWindowHeight();
		createInfo.format = mDepthFormat;
		createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		createInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		createInfo.name = "VulkanBase depth stencil image";
		mDepthStencil = new Image(createInfo, mDevice);
		//mDepthStencil = new Image(mDevice, GetWindowWidth(), GetWindowHeight(),
		//						  mDepthFormat,
		//						  VK_IMAGE_TILING_OPTIMAL,
		//						  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		//						  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		//						  VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		
		mRenderPass = new RenderPass(mDevice, mColorFormat, mDepthFormat, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		mFrameBuffers = new FrameBuffers(mDevice, mRenderPass, mDepthStencil, &mSwapChain, GetWindowWidth(), GetWindowHeight());

		mImageAvailable = std::make_shared<Semaphore>(mDevice);
		mRenderComplete = std::make_shared<Semaphore>(mDevice);
		SetWaitSubmitSemaphore(mImageAvailable);

		mWaitFence = std::make_shared<Fence>(mDevice, 0*VK_FENCE_CREATE_SIGNALED_BIT);
	}

	void VulkanBase::SetupSwapchain()
	{
		// Setup function pointers for the swap chain
		mSwapChain.connect(mInstance->GetVkHandle(), mDevice->GetPhysicalDevice(), mDevice->GetVkDevice());
		mSwapChain.initSurface(mWindow->GetInstance(), mWindow->GetHwnd());

		uint32_t width = GetWindowWidth();
		uint32_t height = GetWindowHeight();
		mSwapChain.create(&width, &height);
	}

	void VulkanBase::PrepareFrame()
	{
		mDevice->GarbageCollect();

		Queue* queue = mDevice->GetQueue();
		Debug::ErrorCheck(mSwapChain.acquireNextImage(GetImageAvailableSemaphore()->GetVkHandle(),
													  &mFrameBuffers->currentFrameBuffer));
	}

	void VulkanBase::SubmitFrame()
	{
		Queue* queue = mDevice->GetQueue();
		Debug::ErrorCheck(mSwapChain.queuePresent(queue->GetVkHandle(),
												  mFrameBuffers->currentFrameBuffer,
												  GetRenderCompleteSemaphore()->GetVkHandle()));
	}

	bool VulkanBase::PreviousFrameComplete()
	{
		static bool firstFrame = true;
		if (firstFrame)
		{
			firstFrame = false;
			return true;
		}

		VkResult fenceStatus = vkGetFenceStatus(mDevice->GetVkDevice(), mWaitFence->GetVkHandle());
		if (fenceStatus == VK_SUCCESS)
		{
			mWaitFence->Reset();
			return true;
		}

		return false;
	}

	Device* VulkanBase::GetDevice()
	{
		return mDevice;
	}

	RenderPass* VulkanBase::GetRenderPass()
	{
		return mRenderPass;
	}

	void VulkanBase::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Game::HandleMessages() handles the closing of the window
	}

	uint32_t VulkanBase::GetWindowWidth()
	{
		return mWindow->GetWidth();
	}

	uint32_t VulkanBase::GetWindowHeight()
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

	const SharedPtr<Semaphore>& VulkanBase::GetImageAvailableSemaphore() const
	{
		return mImageAvailable;
	}

	const SharedPtr<Semaphore>& VulkanBase::GetRenderCompleteSemaphore() const
	{
		return mRenderComplete;
	}

	const SharedPtr<Semaphore>& VulkanBase::GetWaitSubmitSemaphore() const
	{
		return mWaitSubmitSemaphore;
	}

	void VulkanBase::SetWaitSubmitSemaphore(const SharedPtr<Semaphore>& waitSemaphore)
	{
		mWaitSubmitSemaphore = waitSemaphore;
	}
}	// VulkanLib namespace