#include <vector>
#include <array>
#include <cassert>
#include <sstream>
#include <chrono>
#include "Platform.h"
#include "VulkanBase.h"
#include "Device.h"
#include "VulkanDebug.h"
#include "Window.h"
#include "ShaderManager.h"
#include "handles/CommandPool.h"
#include "handles/Semaphore.h"
#include "handles/Image.h"
#include "handles/RenderPass.h"
#include "handles/Instance.h"
#include "handles/FrameBuffers.h"
#include "handles/Queue.h"

/*
-	Right now this code assumes that queueFamilyIndex is = 0 in all places,
no looping is done to find a queue that have the proper support
*/

namespace VulkanLib
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

		// Synchronization code missing here, VkSemaphore etc.
	}

	VulkanBase::~VulkanBase()
	{
		mSwapChain.cleanup();

		// Destroy Vulkan handles
		delete mPresentComplete;
		delete mRenderComplete;
		delete mCommandPool;
		delete mQueue;
		delete mDepthStencil;
		delete mRenderPass;
		delete mFrameBuffers;
		delete mShaderManager;
		delete mDevice;

		VulkanDebug::CleanupDebugging(mInstance->GetVkHandle());

		delete mInstance;
	}

	void VulkanBase::Prepare()
	{
		CompileShaders();				// Compile shaders using batch files
		SetupSwapchain();				// Setup the swap chain with the helper class

		mCommandPool = new CommandPool(GetDevice(), 0);
		mPresentComplete = new Semaphore(mDevice);
		mRenderComplete = new Semaphore(mDevice);
		mQueue = new Queue(mDevice, mPresentComplete, mRenderComplete);

		mDepthStencil = new Image(mDevice, GetWindowWidth(), GetWindowHeight(),
			mDepthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		
		mRenderPass = new RenderPass(mDevice, mColorFormat, mDepthFormat);
		mFrameBuffers = new FrameBuffers(mDevice, mRenderPass, mDepthStencil, &mSwapChain, GetWindowWidth(), GetWindowHeight());
		mShaderManager = new ShaderManager(mDevice);
	}

	void VulkanBase::SetupSwapchain()
	{
		uint32_t width = GetWindowWidth();
		uint32_t height = GetWindowHeight();
		mSwapChain.create(&width, &height);
	}

	void VulkanBase::PrepareFrame()
	{
		// Acquire the next image from the swap chaing
		VulkanDebug::ErrorCheck(mSwapChain.acquireNextImage(mPresentComplete->GetVkHandle(), &mFrameBuffers->mCurrentFrameBuffer));
	}

	void VulkanBase::SubmitFrame()
	{
		VulkanDebug::ErrorCheck(mSwapChain.queuePresent(mQueue->GetVkHandle(), mFrameBuffers->mCurrentFrameBuffer, mRenderComplete->GetVkHandle()));
		mQueue->WaitIdle();
	}

	VkBool32 VulkanBase::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
	{
		// [TODO] Remove use of vkTools::initializers
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.pNext = NULL;
		memAllocInfo.allocationSize = 0;
		memAllocInfo.memoryTypeIndex = 0;

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = NULL;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.size = size;
		bufferCreateInfo.flags = 0;

		VulkanDebug::ErrorCheck(vkCreateBuffer(GetDevice(), &bufferCreateInfo, nullptr, buffer));

		vkGetBufferMemoryRequirements(GetDevice(), *buffer, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		uint32_t tmp;
		memAllocInfo.memoryTypeIndex = mDevice->GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, &tmp); // [NOTE] This is really weird
		VulkanDebug::ErrorCheck(vkAllocateMemory(GetDevice(), &memAllocInfo, nullptr, memory));
		if (data != nullptr)
		{
			void *mapped;
			VulkanDebug::ErrorCheck(vkMapMemory(GetDevice(), *memory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);
			vkUnmapMemory(GetDevice(), *memory);
		}
		VulkanDebug::ErrorCheck(vkBindBufferMemory(GetDevice(), *buffer, *memory, 0));

		return true;
	}

	void VulkanBase::InitSwapchain(Window* window)
	{
		mWindow = window;

		// Platform dependent code to initialize the window surface
#if defined(_WIN32)
		mSwapChain.initSurface(mWindow->GetInstance(), mWindow->GetHwnd());
#elif defined(__linux__)
		mSwapChain.initSurface(mWindow->GetConnection(), mWindow->GetWindow());
#endif
	}

	VkDevice VulkanBase::GetDevice()
	{
		return mDevice->GetVkDevice();
	}

	CommandPool* VulkanBase::GetCommandPool()
	{
		return mCommandPool;
	}

	RenderPass * VulkanBase::GetRenderPass()
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
}	// VulkanLib namespace