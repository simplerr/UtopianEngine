#include "Platform.h"

#include <vector>
#include <array>
#include <cassert>
#include <sstream>
#include <chrono>

#include "VulkanBase.h"
#include "Device.h"
#include "VulkanDebug.h"
#include "../base/vulkanTextureLoader.hpp"
#include "Window.h"
#include "CommandPool.h"
#include "Semaphore.h"
#include "Image.h"
#include "RenderPass.h"
#include "Instance.h"

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

		for (uint32_t i = 0; i < mFrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(GetDevice(), mFrameBuffers[i], nullptr);
		}

		for (auto& shaderModule : mShaderModules)
		{
			vkDestroyShaderModule(GetDevice(), shaderModule, nullptr);
		}

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

		mQueue = new Queue(GetDevice(), mPresentComplete, mRenderComplete);

		mDepthStencil = new Image(mDevice, GetWindowWidth(), GetWindowHeight(),
			mDepthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

		mRenderPass = new RenderPass(mDevice, mColorFormat, mDepthFormat);

		SetupFrameBuffer();				// Setup the frame buffer, it uses the depth stencil buffer, render pass and swap chain
	}

	void VulkanBase::SetupFrameBuffer()
	{
		// The code here depends on the depth stencil buffer, the render pass and the swap chain

		VkImageView attachments[2];
		attachments[1] = mDepthStencil->GetView();

		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = mRenderPass->GetVkHandle();
		createInfo.attachmentCount = 2;
		createInfo.pAttachments = attachments;
		createInfo.width = GetWindowWidth();
		createInfo.height = GetWindowHeight();
		createInfo.layers = 1;

		// Create a frame buffer for each swap chain image
		mFrameBuffers.resize(mSwapChain.imageCount);
		for (uint32_t i = 0; i < mFrameBuffers.size(); i++)
		{
			attachments[0] = mSwapChain.buffers[i].view;
			VulkanDebug::ErrorCheck(vkCreateFramebuffer(GetDevice(), &createInfo, nullptr, &mFrameBuffers[i]));
		}
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
		VulkanDebug::ErrorCheck(mSwapChain.acquireNextImage(mPresentComplete->GetVkHandle(), &mCurrentBuffer));
	}

	void VulkanBase::SubmitFrame()
	{
		VulkanDebug::ErrorCheck(mSwapChain.queuePresent(mQueue->GetVkHandle(), mCurrentBuffer, mRenderComplete->GetVkHandle()));
		mQueue->WaitIdle();
	}

	VkBool32 VulkanBase::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
	{
		// [TODO] Remove use of vkTools::initializers
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = vkTools::initializers::memoryAllocateInfo();
		VkBufferCreateInfo bufferCreateInfo = vkTools::initializers::bufferCreateInfo(usageFlags, size);

		VulkanDebug::ErrorCheck(vkCreateBuffer(GetDevice(), &bufferCreateInfo, nullptr, buffer));

		vkGetBufferMemoryRequirements(GetDevice(), *buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		uint32_t tmp;
		memAlloc.memoryTypeIndex = mDevice->GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, &tmp); // [NOTE] This is really weird
		VulkanDebug::ErrorCheck(vkAllocateMemory(GetDevice(), &memAlloc, nullptr, memory));
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

	VkPipelineShaderStageCreateInfo VulkanBase::LoadShader(std::string fileName, VkShaderStageFlagBits stage)
	{
		VkPipelineShaderStageCreateInfo shaderStage = {};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = stage;
#if defined(__ANDROID__)
		shaderStage.module = vkTools::loadShader(androidApp->activity->assetManager, fileName.c_str(), GetDevice(), stage);
#else
		shaderStage.module = vkTools::loadShader(fileName.c_str(), GetDevice(), stage);		// Uses helper functions (NOTE/TODO)
#endif
		shaderStage.pName = "main";
		assert(shaderStage.module != NULL);
		mShaderModules.push_back(shaderStage.module);		// Add them to the vector so they can be cleaned up
		return shaderStage;
	}

	VkDevice VulkanBase::GetDevice()
	{
		return mDevice->GetVkDevice();
	}

	CommandPool* VulkanBase::GetCommandPool()
	{
		return mCommandPool;
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
}	// VulkanLib namespace