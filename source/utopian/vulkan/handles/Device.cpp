#include <stdexcept>
#include <array>
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Instance.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/CommandPool.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/Debug.h"
#include "core/Log.h"
#include <fstream>

#define VMA_IMPLEMENTATION
#include "../external/vk_mem_alloc.h"

namespace Utopian::Vk
{
	Device::Device(Instance* instance, bool enableValidation)
	{
		mEnabledFeatures.geometryShader = VK_TRUE;
		mEnabledFeatures.shaderTessellationAndGeometryPointSize = VK_TRUE;
		mEnabledFeatures.fillModeNonSolid = VK_TRUE;
		mEnabledFeatures.samplerAnisotropy = VK_TRUE;
		mEnabledFeatures.tessellationShader = VK_TRUE;
		mEnabledFeatures.pipelineStatisticsQuery = VK_TRUE;
		mEnabledFeatures.occlusionQueryPrecise = VK_TRUE;
		mEnabledFeatures.independentBlend = VK_TRUE;
		mEnabledFeatures.fragmentStoresAndAtomics = VK_TRUE;
		mEnabledFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;

		RetrievePhysical(instance);
		RetrieveSupportedExtensions();
		RetrieveQueueFamilyProperites();

		vkGetPhysicalDeviceFeatures(mPhysicalDevice, &mAvailableFeatures);
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mDeviceMemoryProperties);

		CreateLogical(enableValidation);

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = mPhysicalDevice;
		allocatorInfo.device = mDevice;
		allocatorInfo.instance = instance->GetVkHandle();

		vmaCreateAllocator(&allocatorInfo, &mAllocator);

		uint32_t queueFamilyIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		mCommandPool = new CommandPool(this, queueFamilyIndex);
		mQueue = new Queue(this);
	}

	Device::~Device()
	{
		delete mCommandPool;
		delete mQueue;

		GarbageCollect();

		vmaDestroyAllocator(mAllocator);
		vkDestroyDevice(mDevice, nullptr);
	}

	void Device::RetrievePhysical(Instance* instance)
	{
		// Query for the number of GPUs
		uint32_t gpuCount = 0;
		VkResult result = vkEnumeratePhysicalDevices(instance->GetVkHandle(), &gpuCount, NULL);

		if (result != VK_SUCCESS)
			UTO_LOG("vkEnumeratePhysicalDevices failed");

		if (gpuCount < 1)
			UTO_LOG("vkEnumeratePhysicalDevices didn't find any valid devices for Vulkan");

		// Enumerate devices
		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		result = vkEnumeratePhysicalDevices(instance->GetVkHandle(), &gpuCount, physicalDevices.data());

		// Assume that there only is 1 GPU
		mPhysicalDevice = physicalDevices[0];

		vkGetPhysicalDeviceProperties(mPhysicalDevice, &mPhysicalDeviceProperties);

		mVulkanVersion = VulkanVersion(mPhysicalDeviceProperties.apiVersion);

		UTO_LOG("Retrieved physical device supporting Vulkan " + mVulkanVersion.version);
	}

	void Device::RetrieveQueueFamilyProperites()
	{
		// Queue family properties, used for setting up requested queues upon device creation
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);
		mQueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, mQueueFamilyProperties.data());
	}

	uint32_t Device::GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(mQueueFamilyProperties.size()); i++)
		{
			if (mQueueFamilyProperties[i].queueFlags & queueFlags)
			{
				return i;
			}
		}

		// No queue was found
		assert(0);
		return 0;
	}

	void Device::CreateLogical(bool enableValidation)
	{
		// Some implementations use vkGetPhysicalDeviceQueueFamilyProperties and uses the result to find out
		// the first queue that support graphic operations (queueFlags & VK_QUEUE_GRAPHICS_BIT)
		// Here I simply set queueInfo.queueFamilyIndex = 0 and (hope) it works
		// In Sascha Willems examples he has a compute queue with queueFamilyIndex = 1

		std::array<float, 1> queuePriorities = { 1.0f };
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pNext = nullptr;
		queueInfo.flags = 0;
		queueInfo.queueFamilyIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		queueInfo.pQueuePriorities = queuePriorities.data();
		queueInfo.queueCount = 1;

		// VK_KHR_SWAPCHAIN_EXTENSION_NAME always needs to be used
		std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		if (IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
		{
			enabledExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
		}

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = nullptr;
		deviceInfo.flags = 0;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.pEnabledFeatures = &mEnabledFeatures;
		deviceInfo.enabledExtensionCount = enabledExtensions.size();
		deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();

		if (enableValidation)
		{
			deviceInfo.enabledLayerCount = Debug::validation_layers.size();
			deviceInfo.ppEnabledLayerNames = Debug::validation_layers.data();
		}
		
		Debug::ErrorCheck(vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice));
	}

	void Device::RetrieveSupportedExtensions()
	{
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (auto ext : extensions)
				{
					mSupportedExtensions.push_back(ext.extensionName);
				}
			}
		}
	}

	bool Device::IsExtensionSupported(std::string extension)
	{
		return (std::find(mSupportedExtensions.begin(), mSupportedExtensions.end(), extension) != mSupportedExtensions.end());
	}

	bool Device::IsDebugMarkersEnabled() const
	{
		return mDebugMarkersEnabled;
	}

	VkPhysicalDevice Device::GetPhysicalDevice() const
	{
		return mPhysicalDevice;
	}

	VkDevice Device::GetVkDevice() const
	{
		return mDevice;
	}

	VkBool32 Device::GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t* typeIndex) const
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

	VkPhysicalDeviceMemoryProperties Device::GetMemoryProperties() const
	{
		return mDeviceMemoryProperties;
	}

	Queue* Device::GetQueue() const
	{
		return mQueue;
	}

	void Device::QueueDestroy(SharedPtr<Vk::Buffer>& buffer)
	{
		mBuffersToFree.push_back(buffer);
		buffer = nullptr;
	}

	void Device::QueueDestroy(VkPipeline pipeline)
	{
		mPipelinesToFree.push_back(pipeline);
    }

	void Device::GarbageCollect()
	{
		if (mBuffersToFree.size() > 0)
		{
			mBuffersToFree.clear();
		}

		for (auto& pipeline : mPipelinesToFree)
			vkDestroyPipeline(GetVkDevice(), pipeline, nullptr);

		if (mPipelinesToFree.size() > 0)
			mPipelinesToFree.clear();
	}

	VmaAllocation Device::AllocateMemory(Image* image, VkMemoryPropertyFlags flags)
	{
		VmaAllocationCreateInfo allocCI = {};
		allocCI.requiredFlags = flags;
		allocCI.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
		std::string imageName = image->GetDebugName();
		allocCI.pUserData = (void*)imageName.c_str();

		VmaAllocationInfo allocInfo;
		VmaAllocation allocation;
		Debug::ErrorCheck(vmaAllocateMemoryForImage(mAllocator, image->GetVkHandle(), &allocCI, &allocation, &allocInfo));
		Debug::ErrorCheck(vkBindImageMemory(mDevice, image->GetVkHandle(), allocInfo.deviceMemory, allocInfo.offset));

		return allocation;
	}

	VmaAllocation Device::AllocateMemory(Buffer* buffer, VkMemoryPropertyFlags flags)
	{
		VmaAllocationCreateInfo allocCI = {};
		allocCI.requiredFlags = flags;
		allocCI.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
		std::string bufferName = buffer->GetDebugName();
		allocCI.pUserData = (void*)bufferName.c_str();

		VmaAllocationInfo allocInfo;
		VmaAllocation memory;
		Debug::ErrorCheck(vmaAllocateMemoryForBuffer(mAllocator, buffer->GetVkHandle(), &allocCI, &memory, &allocInfo));

		Debug::ErrorCheck(vkBindBufferMemory(mDevice, buffer->GetVkHandle(), allocInfo.deviceMemory, allocInfo.offset));

		return memory;
	}

	void Device::MapMemory(VmaAllocation allocation, void** data)
	{
		Debug::ErrorCheck(vmaMapMemory(mAllocator, allocation, data));
	}

	void Device::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(mAllocator, allocation);
	}

	void Device::FreeMemory(VmaAllocation allocation)
	{
		vmaFreeMemory(mAllocator, allocation);
	}

	void Device::GetAllocationInfo(VmaAllocation allocation, VkDeviceMemory& memory, VkDeviceSize& offset)
	{
		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(mAllocator, allocation, &allocInfo);

		memory = allocInfo.deviceMemory;
		offset = allocInfo.offset;
	}

	VmaBudget Device::GetMemoryBudget(VkMemoryHeapFlags heapFlags)
	{
		std::vector<VmaBudget> budget(mDeviceMemoryProperties.memoryHeapCount);

		vmaGetBudget(mAllocator, budget.data());
		uint32_t deviceMemoryUsage = 0u;
		VmaBudget totalBudget = { 0u };
		for (uint32_t i = 0; i < mDeviceMemoryProperties.memoryHeapCount; i++)
		{
			if ((mDeviceMemoryProperties.memoryHeaps[i].flags & heapFlags) != 0)
			{
				totalBudget.blockBytes += budget[i].blockBytes;
				totalBudget.allocationBytes += budget[i].allocationBytes;
				totalBudget.usage += budget[i].usage;
				totalBudget.budget += budget[i].budget;
			}
			deviceMemoryUsage += budget[i].usage;
		}

		return totalBudget;
	}

	void Device::GetMemoryStats(VmaStats* stats)
	{
		vmaCalculateStats(mAllocator, stats);
	}

	void Device::DumpMemoryStats(std::string filename)
	{
		char* statsPtr;
		vmaBuildStatsString(mAllocator, &statsPtr, true);

		std::string statsString = statsPtr;
		std::ofstream fout(filename);

		fout << statsString;

		fout.close();
		vmaFreeStatsString(mAllocator, statsPtr);
	}

	CommandPool* Device::GetCommandPool() const
	{
		return mCommandPool;
	}

	VulkanVersion Device::GetVulkanVersion() const
	{
		return mVulkanVersion;
	}

	VulkanVersion::VulkanVersion()
	{

	}

	VulkanVersion::VulkanVersion(uint32_t apiVersion)
	{
		major = VK_VERSION_MAJOR(apiVersion);
		minor = VK_VERSION_MINOR(apiVersion);
		patch = VK_VERSION_PATCH(apiVersion);

		version = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
	}
}
