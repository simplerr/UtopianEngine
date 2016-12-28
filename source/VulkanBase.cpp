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

		delete mTextureLoader;

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
		CreateSetupCommandBuffer();		// Create the setup command buffer used for queuing initialization command, also starts recording to the setup command buffer with vkBeginCommandBuffer
		SetupSwapchain();				// Setup the swap chain with the helper class
		CreateSemaphores();
		CreateCommandBuffers();			// Create the command buffers used for drawing and the image format transitions
		BuildPresentCommandBuffers();
		SetupDepthStencil();			// Setup the depth stencil buffer
		SetupRenderPass();				// Setup the render pass
		SetupFrameBuffer();				// Setup the frame buffer, it uses the depth stencil buffer, render pass and swap chain
		ExecuteSetupCommandBuffer();	// Submit all commands so far to the queue, end and free the setup command buffer
		CreateSetupCommandBuffer();		// The derived class will also record initialization commands to the setup command buffer

										// Create a simple texture loader class
		mTextureLoader = new vkTools::VulkanTextureLoader(mDevice, mQueue, mCommandPool);

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
		VkResult r = vkCreateCommandPool(mDevice, &createInfo, nullptr, &mCommandPool);
		assert(!r);
	}

	void VulkanBase::CreateSetupCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkResult r = vkAllocateCommandBuffers(mDevice, &allocateInfo, &mSetupCmdBuffer);
		assert(!r);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//beginInfo.flags = Default is OK, change this if multiple command buffers (primary & secondary)

		// Begin recording commands to the setup command buffer
		r = vkBeginCommandBuffer(mSetupCmdBuffer, &beginInfo);
		assert(!r);
	}

	void VulkanBase::CreateCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		// Allocate the command buffers that the image memory barriers will use to change the swap chain image format
		//VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, &mPrePresentCmdBuffer));
		//VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, &mPostPresentCmdBuffer));

		// Allocate a command buffer for each swap chain image
		mRenderingCommandBuffers.resize(mSwapChain.imageCount);
		mPrePresentCmdBuffers.resize(mSwapChain.imageCount);
		mPostPresentCmdBuffers.resize(mSwapChain.imageCount);

		allocateInfo.commandBufferCount = mRenderingCommandBuffers.size();

		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, mRenderingCommandBuffers.data()));

		// Command buffers for submitting present barriers
		// One pre and post present buffer per swap chain image
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, mPrePresentCmdBuffers.data()));
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, mPostPresentCmdBuffers.data()));
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

		VkResult res = vkCreateImage(mDevice, &imageCreateInfo, nullptr, &mDepthStencil.image);
		assert(!res);

		// Get memory requirements
		VkMemoryRequirements memRequirments;
		vkGetImageMemoryRequirements(mDevice, mDepthStencil.image, &memRequirments);
		allocateInfo.allocationSize = memRequirments.size;
		GetMemoryType(memRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocateInfo.memoryTypeIndex);

		res = vkAllocateMemory(mDevice, &allocateInfo, nullptr, &mDepthStencil.memory);
		assert(!res);

		res = vkBindImageMemory(mDevice, mDepthStencil.image, mDepthStencil.memory, 0);
		assert(!res);

		// [NOTE] This is removed in the latest Sascha Willems code
		vkTools::setImageLayout(mSetupCmdBuffer, mDepthStencil.image, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		viewCreateInfo.image = mDepthStencil.image;	// Connect the view with the image
		res = vkCreateImageView(mDevice, &viewCreateInfo, nullptr, &mDepthStencil.view);
		assert(!res);
	}

	void VulkanBase::SetupRenderPass()
	{
		// Subpass creation, standard code 100% from Vulkan samples
		// Also specifices which attachments from pSubpasses to use
		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = NULL;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;
		subpass.pResolveAttachments = NULL;
		subpass.pDepthStencilAttachment = &depthReference;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = NULL;

		// Attachments creation, standard code 100% from Vulkan samples
		// Basically creates one color attachment and one depth stencil attachment
		VkAttachmentDescription attachments[2];
		attachments[0].format = mColorFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachments[1].format = mDepthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkRenderPassCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.attachmentCount = 2;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.pAttachments = attachments;
		
		// [TODO] Major modifications here 
		// It seems like the memory barrier transitions are replaced with render pass dependencies

		VkResult res = vkCreateRenderPass(mDevice, &createInfo, nullptr, &mRenderPass);
		assert(!res);
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
			VkResult res = vkCreateFramebuffer(mDevice, &createInfo, nullptr, &mFrameBuffers[i]);
			assert(!res);
		}
	}

	void VulkanBase::SetupSwapchain()
	{
		// Note that we use the same command buffer for everything right now!
		// Uses the setup command buffer
		uint32_t width = GetWindowWidth();
		uint32_t height = GetWindowHeight();
		mSwapChain.create(&width, &height);
	}

	void VulkanBase::ExecuteSetupCommandBuffer()
	{
		if (mSetupCmdBuffer == VK_NULL_HANDLE)
			return;

		VkResult err = vkEndCommandBuffer(mSetupCmdBuffer);
		assert(!err);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mSetupCmdBuffer;

		err = vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
		assert(!err);

		err = vkQueueWaitIdle(mQueue);
		assert(!err);

		vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mSetupCmdBuffer);
		mSetupCmdBuffer = VK_NULL_HANDLE;
	}

	void VulkanBase::PrepareFrame()
	{
		// Acquire the next image from the swap chaing
		VulkanDebug::ErrorCheck(mSwapChain.acquireNextImage(mPresentComplete, &mCurrentBuffer));

		// Submit post present image barrier to transform the image back to a color attachment that our render pass can write to
		VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mPostPresentCmdBuffers[mCurrentBuffer];
		VulkanDebug::ErrorCheck(vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE));
	}

	void VulkanBase::SubmitFrame()
	{
		// Submit pre present image barrier to transform the image from color attachment to present(khr) for presenting to the swap chain
		VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mPrePresentCmdBuffers[mCurrentBuffer];
		VulkanDebug::ErrorCheck(vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE));

		VulkanDebug::ErrorCheck(mSwapChain.queuePresent(mQueue, mCurrentBuffer, mRenderComplete));

		VulkanDebug::ErrorCheck(vkQueueWaitIdle(mQueue));
	}

	void VulkanBase::SubmitPrePresentMemoryBarrier(VkImage image)
	{
		// Copy & paste, I think this can be done smarter (TODO)
		//VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

		//VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mPrePresentCmdBuffer, &cmdBufInfo));

		//VkImageMemoryBarrier prePresentBarrier = vkTools::initializers::imageMemoryBarrier();
		//prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		//prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		//prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;						// New layout for presenting
		//prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		//prePresentBarrier.image = image;

		//vkCmdPipelineBarrier(
		//	mPrePresentCmdBuffer,
		//	VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		//	VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		//	VK_FLAGS_NONE,
		//	0, nullptr, // No memory barriers,
		//	0, nullptr, // No buffer barriers,
		//	1, &prePresentBarrier);

		//VulkanDebug::ErrorCheck(vkEndCommandBuffer(mPrePresentCmdBuffer));

		//VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
		//submitInfo.commandBufferCount = 1;
		//submitInfo.pCommandBuffers = &mPrePresentCmdBuffer;

		//VulkanDebug::ErrorCheck(vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE));
	}

	void VulkanBase::SubmitPostPresentMemoryBarrier(VkImage image)
	{
		// Copy & paste, I think this can be done smarter (TODO)
		//VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

		//VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mPostPresentCmdBuffer, &cmdBufInfo));

		//VkImageMemoryBarrier postPresentBarrier = vkTools::initializers::imageMemoryBarrier();	// TODO: Remove VkTools code
		//postPresentBarrier.srcAccessMask = 0;
		//postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		//postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;				// New layout for rendering
		//postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		//postPresentBarrier.image = image;

		//vkCmdPipelineBarrier(
		//	mPostPresentCmdBuffer,
		//	VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		//	VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		//	0,
		//	0, nullptr, // No memory barriers,
		//	0, nullptr, // No buffer barriers,
		//	1, &postPresentBarrier);

		//VulkanDebug::ErrorCheck(vkEndCommandBuffer(mPostPresentCmdBuffer));

		//VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
		//submitInfo.commandBufferCount = 1;
		//submitInfo.pCommandBuffers = &mPostPresentCmdBuffer;

		//VulkanDebug::ErrorCheck(vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE));
	}

	VkBool32 VulkanBase::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
	{
		//VkBufferCreateInfo createInfo = {};
		//VkMemoryAllocateInfo allocInfo = {};
		//VkMemoryRequirements memoryRequirments = {};

		//allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		//allocInfo.allocationSize = 0;								// Gets assigned with vkGetBufferMemoryRequirements
		//allocInfo.memoryTypeIndex = 0;

		//createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

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

	// [NOTE] This entire function is removed in the latest Sascha Willems code
	void VulkanBase::BuildPresentCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

		for (uint32_t i = 0; i < mSwapChain.imageCount; i++)
		{
			// Command buffer for post present barrier

			// Insert a post present image barrier to transform the image back to a
			// color attachment that our render pass can write to
			// We always use undefined image layout as the source as it doesn't actually matter
			// what is done with the previous image contents

			VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mPostPresentCmdBuffers[i], &cmdBufInfo));

			VkImageMemoryBarrier postPresentBarrier = vkTools::initializers::imageMemoryBarrier();
			postPresentBarrier.srcAccessMask = 0;
			postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			postPresentBarrier.image = mSwapChain.buffers[i].image;

			vkCmdPipelineBarrier(
				mPostPresentCmdBuffers[i],
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &postPresentBarrier);

			VulkanDebug::ErrorCheck(vkEndCommandBuffer(mPostPresentCmdBuffers[i]));

			// Command buffers for pre present barrier

			// Submit a pre present image barrier to the queue
			// Transforms the (framebuffer) image layout from color attachment to present(khr) for presenting to the swap chain

			VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mPrePresentCmdBuffers[i], &cmdBufInfo));

			VkImageMemoryBarrier prePresentBarrier = vkTools::initializers::imageMemoryBarrier();
			prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			prePresentBarrier.image = mSwapChain.buffers[i].image;

			vkCmdPipelineBarrier(
				mPrePresentCmdBuffers[i],
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_FLAGS_NONE,
				0, nullptr, // No memory barriers,
				0, nullptr, // No buffer barriers,
				1, &prePresentBarrier);

			VulkanDebug::ErrorCheck(vkEndCommandBuffer(mPrePresentCmdBuffers[i]));
		}
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

#if defined(_WIN32)
	void VulkanBase::RenderLoop()
	{
		int a = 1;
		/*MSG msg;
		<<
		while (true)
		{
		// Frame begin
		mTimer.FrameBegin();

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
		if (msg.message == WM_QUIT)
		{
		break;
		}
		else
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
		}

		Update();
		Render();

		// Frame end
		auto fps = mTimer.FrameEnd();

		// Only display fps when 1.0s have passed
		if (fps != -1)
		{
		std::string windowTitle = "Project Vulkan: " + std::to_string(fps) + " fps";
		SetWindowText(mWindow->GetHwnd(), windowTitle.c_str());
		}
		}*/
	}
#endif

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
		switch (msg)
		{
		case WM_CLOSE:
			DestroyWindow(mWindow->GetHwnd());
			PostQuitMessage(0);
			break;
		}
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