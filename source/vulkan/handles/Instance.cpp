#include <vector>
#include "vulkan/Debug.h"
#include "Instance.h"

namespace Utopian::Vk
{
	Instance::Instance(std::string appName, bool enableValidation)
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = appName.c_str();
		appInfo.pEngineName = appName.c_str();
		appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

		std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

		// Extension for the Win32 surface 
#if defined(_WIN32)
		enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
		enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
		enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

		if (IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		if (IsExtensionSupported(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME))
			enabledExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = &Debug::debugUtilsCreateInfo;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = enabledExtensions.size();
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();

		if (enableValidation)
		{
			createInfo.enabledLayerCount = Debug::validation_layers.size();
			createInfo.ppEnabledLayerNames = Debug::validation_layers.data();
		}

		Debug::ErrorCheck(vkCreateInstance(&createInfo, NULL, &mInstance));
	}

	Instance::~Instance()
	{
		vkDestroyInstance(mInstance, nullptr);
	}

	bool Instance::IsExtensionSupported(std::string extension) const
	{
		uint32_t instanceExtensionCount;
		Debug::ErrorCheck(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));

		std::vector<VkExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
		Debug::ErrorCheck(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableInstanceExtensions.data()));

		bool supported = false;
		for (auto& available_extension : availableInstanceExtensions)
		{
			if (strcmp(available_extension.extensionName, extension.c_str()) == 0)
			{
				supported = true;
			}
		}

		if (!supported)
			Debug::ConsolePrint(extension + " is not supported.");

		return supported;
	}

	VkInstance Instance::GetVkHandle()
	{
		return mInstance;
	}
}