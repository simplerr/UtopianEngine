#include <stdexcept>
#include <array>
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Instance.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/CommandPool.h"
#include "vulkan/Debug.h"

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

		RetrievePhysical(instance);
		RetrieveSupportedExtensions();
		RetrieveQueueFamilyProperites();
		CreateLogical(enableValidation);

		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mDeviceMemoryProperties);

		uint32_t queueFamilyIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		mCommandPool = new CommandPool(this, queueFamilyIndex);
		mQueue = new Queue(this);
	}

	Device::~Device()
	{
		delete mCommandPool;
		delete mQueue;

		vkDestroyDevice(mDevice, nullptr);
	}

	void Device::RetrievePhysical(Instance* instance)
	{
		// Query for the number of GPUs
		uint32_t gpuCount = 0;
		VkResult result = vkEnumeratePhysicalDevices(instance->GetVkHandle(), &gpuCount, NULL);

		if (result != VK_SUCCESS)
			Debug::ConsolePrint("vkEnumeratePhysicalDevices failed");

		if (gpuCount < 1)
			Debug::ConsolePrint("vkEnumeratePhysicalDevices didn't find any valid devices for Vulkan");

		// Enumerate devices
		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		result = vkEnumeratePhysicalDevices(instance->GetVkHandle(), &gpuCount, physicalDevices.data());

		// Assume that there only is 1 GPU
		mPhysicalDevice = physicalDevices[0];
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
	}

	void Device::CreateLogical(bool enableValidation)
	{
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
		queueInfo.queueFamilyIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		queueInfo.pQueuePriorities = queuePriorities.data();
		queueInfo.queueCount = 1;

		// VK_KHR_SWAPCHAIN_EXTENSION_NAME allways needs to be used
		std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
		if (IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
		{
			// Note: Todo: Enabling debug markers seems to break RenderDoc.
			enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			mDebugMarkersEnabled = true;
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
			deviceInfo.enabledLayerCount = Debug::validation_layers.size();	// Debug validation layers
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

	VkPhysicalDeviceMemoryProperties Device::GetPhysicalDeviceMemoryProperties() const
	{
		return mDeviceMemoryProperties;
	}

	Queue* Device::GetQueue() const
	{
		return mQueue;
	}

	CommandPool* Device::GetCommandPool() const
	{
		return mCommandPool;
	}
}
