#include "Platform.h"

#include <vector>
#include <array>
#include <cassert>
#include <sstream>
#include <chrono>

#include "VulkanBase.h"
#include "VulkanDebug.h"
#include "../base/vulkanTextureLoader.hpp"
#include "Window.h"

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
		VulkanDebug::ErrorCheck(CreateInstance("Vulkan App", enableValidation));

		VulkanDebug::InitDebug(mInstance);

		// Create VkDevice
		VulkanDebug::ErrorCheck(CreateDevice(enableValidation));

		// Get the graphics queue
		vkGetDeviceQueue(mDevice, 0, 0, &mQueue);	// Note that queueFamilyIndex is hard coded to 0

		// Gather physical device memory properties
		// [TODO] This should be moved to a VulkanDevice struct
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mDeviceMemoryProperties);

		// Setup function pointers for the swap chain
		mSwapChain.connect(mInstance, mPhysicalDevice, mDevice);

		// Synchronization code missing here, VkSemaphore etc.
	}

	VulkanBase::~VulkanBase()
	{
		mSwapChain.cleanup();

		// Destroy semaphores
		vkDestroySemaphore(mDevice, mPresentComplete, nullptr);
		vkDestroySemaphore(mDevice, mRenderComplete, nullptr);

		//delete mTextureLoader;

		vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

		// Cleanup depth stencil data
		vkDestroyImageView(mDevice, mDepthStencil.view, nullptr);
		vkDestroyImage(mDevice, mDepthStencil.image, nullptr);
		vkFreeMemory(mDevice, mDepthStencil.memory, nullptr);

		vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

		for (uint32_t i = 0; i < mFrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(mDevice, mFrameBuffers[i], nullptr);
		}

		for (auto& shaderModule : mShaderModules)
		{
			vkDestroyShaderModule(mDevice, shaderModule, nullptr);
		}

		vkDestroyDevice(mDevice, nullptr);

		VulkanDebug::CleanupDebugging(mInstance);

		vkDestroyInstance(mInstance, nullptr);
	}

	void VulkanBase::Prepare()
	{
		CompileShaders();				// Compile shaders using batch files
		CreateCommandPool();			// Create a command pool to allocate command buffers from
		SetupSwapchain();				// Setup the swap chain with the helper class
		CreateSemaphores();
		CreateCommandBuffers();			// Create the command buffers used for drawing and the image format transitions
		SetupDepthStencil();			// Setup the depth stencil buffer
		SetupRenderPass();				// Setup the render pass
		SetupFrameBuffer();				// Setup the frame buffer, it uses the depth stencil buffer, render pass and swap chain

		// Create a simple texture loader class
		//mTextureLoader = new vkTools::VulkanTextureLoader(mDevice, mQueue, mCommandPool);

		// The derived class initializes:
		// Pipeline
		// Uniform buffers
		// Vertex buffers 
		// Descriptor sets
	}

	VkResult VulkanBase::CreateInstance(const char* appName, bool enableValidation)
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			// Must be VK_STRUCTURE_TYPE_APPLICATION_INFO
		appInfo.pNext = nullptr;									// Must be NULL
		appInfo.pApplicationName = appName;
		appInfo.pEngineName = appName;
		appInfo.apiVersion = VK_API_VERSION_1_0;				// All drivers support this, but use VK_API_VERSION in the future

		std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

		// Extension for the Win32 surface 
#if defined(_WIN32)
		enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
		enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
		enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

		// Add the debug extension
		enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// Must be VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
		createInfo.pNext = &VulkanDebug::debugCallbackCreateInfo;	// Enables debugging when creating the instance
		createInfo.flags = 0;										// Must be 0
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = enabledExtensions.size();			// Extensions
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();

		if (enableValidation)
		{
			createInfo.enabledLayerCount = VulkanDebug::validation_layers.size();	// Debug validation layers
			createInfo.ppEnabledLayerNames = VulkanDebug::validation_layers.data();
		}

		VkResult res = vkCreateInstance(&createInfo, NULL, &mInstance);

		return res;
	}

	VkResult VulkanBase::CreateDevice(bool enableValidation)
	{
		// Query for the number of GPUs
		uint32_t gpuCount = 0;
		VkResult result = vkEnumeratePhysicalDevices(mInstance, &gpuCount, NULL);

		if (result != VK_SUCCESS)
			VulkanDebug::ConsolePrint("vkEnumeratePhysicalDevices failed");

		if (gpuCount < 1)
			VulkanDebug::ConsolePrint("vkEnumeratePhysicalDevices didn't find any valid devices for Vulkan");

		// Enumerate devices
		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		result = vkEnumeratePhysicalDevices(mInstance, &gpuCount, physicalDevices.data());

		// Assume that there only is 1 GPU
		mPhysicalDevice = physicalDevices[0];

		// This is not used right now, but GPU vendor and model can be retrieved
		//VkPhysicalDeviceProperties physicalDeviceProperties;
		//vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		// Some implementations use vkGetPhysicalDeviceQueueFamilyProperties and uses the result to find out
		// the first queue that support graphic operations (queueFlags & VK_QUEUE_GRAPHICS_BIT)
		// Here I simply set queueInfo.queueFamilyIndex = 0 and (hope) it works
		// In Sascha Willems examples he has a compute queue with queueFamilyIndex = 1

		std::array<float, 1> queuePriorities = { 1.0f };
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pNext = nullptr;
		queueInfo.flags = 0;
		queueInfo.queueFamilyIndex = 0; // 0 seems to always be the first valid queue (see above)
		queueInfo.pQueuePriorities = queuePriorities.data();
		queueInfo.queueCount = 1;

		// Use the VK_KHR_SWAPCHAIN_EXTENSION_NAME extension
		// [NOTE] There is another VK_EXT_DEBUG_MARKER_EXTENSION_NAME extension that might be good to add later.
		std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = nullptr;
		deviceInfo.flags = 0;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.pEnabledFeatures = nullptr;
		deviceInfo.enabledExtensionCount = enabledExtensions.size();				// Extensions
		deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();

		if (enableValidation)
		{
			deviceInfo.enabledLayerCount = VulkanDebug::validation_layers.size();	// Debug validation layers
			deviceInfo.ppEnabledLayerNames = VulkanDebug::validation_layers.data();
		}

		result = vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice);

		return result;
	}

	void VulkanBase::CreateCommandPool()
	{
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = 0;									// NOTE: TODO: Need to store this as a member (Use Swapchain)!!!!!
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VulkanDebug::ErrorCheck(vkCreateCommandPool(mDevice, &createInfo, nullptr, &mCommandPool));
	}

	void VulkanBase::CreateCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		// Allocate a command buffer for each swap chain image
		mRenderingCommandBuffers.resize(mSwapChain.imageCount);

		allocateInfo.commandBufferCount = mRenderingCommandBuffers.size();

		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, mRenderingCommandBuffers.data()));
	}

	void VulkanBase::CreateSemaphores()
	{
		VkSemaphoreCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// Create a semaphore used to synchronize image presentation
		// Ensures that the image is displayed before we start submitting new commands to the queu
		VulkanDebug::ErrorCheck(vkCreateSemaphore(mDevice, &createInfo, nullptr, &mPresentComplete));

		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands have been sumbitted and executed
		VulkanDebug::ErrorCheck(vkCreateSemaphore(mDevice, &createInfo, nullptr, &mRenderComplete));
	}

	VkCommandBuffer VulkanBase::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		VkCommandBuffer cmdBuffer;
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = level;

		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, &cmdBuffer));

		// If requested, also start the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VulkanDebug::ErrorCheck(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
		}

		return cmdBuffer;
	}

	void VulkanBase::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		VulkanDebug::ErrorCheck(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(queue));

		if (free)
		{
			vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
		}
	}

	void VulkanBase::SetupDepthStencil()
	{
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.format = mDepthFormat;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent = { (uint32_t)GetWindowWidth(), (uint32_t)GetWindowHeight(), 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		// The rest can have their default 0 value

		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = mDepthFormat;
		viewCreateInfo.subresourceRange = {};
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		VulkanDebug::ErrorCheck(vkCreateImage(mDevice, &imageCreateInfo, nullptr, &mDepthStencil.image));

		// Get memory requirements
		VkMemoryRequirements memRequirments;
		vkGetImageMemoryRequirements(mDevice, mDepthStencil.image, &memRequirments);
		allocateInfo.allocationSize = memRequirments.size;
		allocateInfo.memoryTypeIndex = 0; // [NOTE] 0 seems to do fine, but proper way is to use getMemoryTypeIndex()
		GetMemoryType(memRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocateInfo.memoryTypeIndex);

		VulkanDebug::ErrorCheck(vkAllocateMemory(mDevice, &allocateInfo, nullptr, &mDepthStencil.memory));
		VulkanDebug::ErrorCheck(vkBindImageMemory(mDevice, mDepthStencil.image, mDepthStencil.memory, 0));

		// Connect the view with the image
		viewCreateInfo.image = mDepthStencil.image;	
		VulkanDebug::ErrorCheck(vkCreateImageView(mDevice, &viewCreateInfo, nullptr, &mDepthStencil.view));
	}

	void VulkanBase::SetupRenderPass()
	{
		// Descriptors for the attachments used by this renderpass
		std::array<VkAttachmentDescription, 2> attachments = {};

		// Color attachment
		attachments[0].format = mColorFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;									
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;						
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
																	
		// Depth attachment											
		attachments[1].format = mDepthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;						
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	

		// Setup attachment references																			
		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;											
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;									
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		

		// Setup a single subpass reference																			
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;							
		subpassDescription.pColorAttachments = &colorReference;				
		subpassDescription.pDepthStencilAttachment = &depthReference;	
		subpassDescription.inputAttachmentCount = 0;				
		subpassDescription.pInputAttachments = nullptr;			
		subpassDescription.preserveAttachmentCount = 0;		
		subpassDescription.pPreserveAttachments = nullptr;								
		subpassDescription.pResolveAttachments = nullptr;							

		std::array<VkSubpassDependency, 2> dependencies;

		// First dependency at the start of the renderpass
		// Does the transition from final to initial layout 
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						
		dependencies[0].dstSubpass = 0;										
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Second dependency at the end the renderpass
		// Does the transition from the initial to the final layout
		dependencies[1].srcSubpass = 0;									
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;			
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());	
		renderPassInfo.pAttachments = attachments.data();						
		renderPassInfo.subpassCount = 1;									
		renderPassInfo.pSubpasses = &subpassDescription;				
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());	
		renderPassInfo.pDependencies = dependencies.data();							

		VulkanDebug::ErrorCheck(vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass));
	}

	void VulkanBase::SetupFrameBuffer()
	{
		// The code here depends on the depth stencil buffer, the render pass and the swap chain

		VkImageView attachments[2];
		attachments[1] = mDepthStencil.view;

		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = mRenderPass;
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
			VulkanDebug::ErrorCheck(vkCreateFramebuffer(mDevice, &createInfo, nullptr, &mFrameBuffers[i]));
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
		VulkanDebug::ErrorCheck(mSwapChain.acquireNextImage(mPresentComplete, &mCurrentBuffer));
	}

	void VulkanBase::SubmitFrame()
	{
		VulkanDebug::ErrorCheck(mSwapChain.queuePresent(mQueue, mCurrentBuffer, mRenderComplete));
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(mQueue));
	}

	VkBool32 VulkanBase::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
	{
		// [TODO] Remove use of vkTools::initializers
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = vkTools::initializers::memoryAllocateInfo();
		VkBufferCreateInfo bufferCreateInfo = vkTools::initializers::bufferCreateInfo(usageFlags, size);

		VulkanDebug::ErrorCheck(vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, buffer));

		vkGetBufferMemoryRequirements(mDevice, *buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		uint32_t tmp;
		memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, &tmp);
		VulkanDebug::ErrorCheck(vkAllocateMemory(mDevice, &memAlloc, nullptr, memory));
		if (data != nullptr)
		{
			void *mapped;
			VulkanDebug::ErrorCheck(vkMapMemory(mDevice, *memory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);
			vkUnmapMemory(mDevice, *memory);
		}
		VulkanDebug::ErrorCheck(vkBindBufferMemory(mDevice, *buffer, *memory, 0));

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
		shaderStage.module = vkTools::loadShader(androidApp->activity->assetManager, fileName.c_str(), mDevice, stage);
#else
		shaderStage.module = vkTools::loadShader(fileName.c_str(), mDevice, stage);		// Uses helper functions (NOTE/TODO)
#endif
		shaderStage.pName = "main";
		assert(shaderStage.module != NULL);
		mShaderModules.push_back(shaderStage.module);		// Add them to the vector so they can be cleaned up
		return shaderStage;
	}

	VkDevice VulkanBase::GetDevice()
	{
		return mDevice;
	}

	// Code from Vulkan samples and SaschaWillems
	VkBool32 VulkanBase::GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex)
	{
		for (uint32_t i = 0; i < 32; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((mDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					*typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}
		return false;
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