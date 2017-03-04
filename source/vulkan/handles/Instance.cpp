#include <vector>
#include "vulkan/VulkanDebug.h"
#include "Instance.h"

namespace Vulkan
{
	Instance::Instance(std::string appName, bool enableValidation)
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			// Must be VK_STRUCTURE_TYPE_APPLICATION_INFO
		appInfo.pNext = nullptr;									// Must be NULL
		appInfo.pApplicationName = appName.c_str();
		appInfo.pEngineName = appName.c_str();
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

		VulkanDebug::ErrorCheck(vkCreateInstance(&createInfo, NULL, &mInstance));
	}

	Instance::~Instance()
	{
		vkDestroyInstance(mInstance, nullptr);
	}

	VkInstance Instance::GetVkHandle()
	{
		return mInstance;
	}
}