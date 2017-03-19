#include <sstream>
#include <iostream>
#include <cassert>
#include "VulkanDebug.h"

namespace Vulkan
{
	namespace VulkanDebug
	{
		// Debug extension callbacks
		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = nullptr;
		PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = nullptr;
		PFN_vkDebugReportMessageEXT dbgBreakCallback = nullptr;

		std::vector<const char*> validation_layers;
		VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
		VkDebugReportCallbackEXT msgCallback = nullptr;
		bool performanceWarnings = true;

		void SetupDebugLayers()
		{
			// Create a console to forward standard output to
			SetupConsole("Vulkan Debug Console");

			// TODO: Add #ifdef _DEBUG
			// Configure so that VulkanDebug::VulkanDebugCallback() gets all debug messages
			debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			debugCallbackCreateInfo.pNext = nullptr;
			debugCallbackCreateInfo.flags =
				//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
				//VK_DEBUG_REPORT_DEBUG_BIT_EXT |
				VK_DEBUG_REPORT_ERROR_BIT_EXT |
				VK_DEBUG_REPORT_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

			debugCallbackCreateInfo.pfnCallback = &VulkanDebugCallback;
			debugCallbackCreateInfo.pUserData = nullptr;

			// Add all standard validation layers http://gpuopen.com/using-the-vulkan-validation-layers/
			validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");

			/*validation_layers.push_back("VK_LAYER_GOOGLE_threading");
			validation_layers.push_back("VK_LAYER_LUNARG_mem_tracker");
			validation_layers.push_back("VK_LAYER_LUNARG_object_tracker");
			validation_layers.push_back("VK_LAYER_LUNARG_draw_state");
			validation_layers.push_back("VK_LAYER_LUNARG_param_checker");
			validation_layers.push_back("VK_LAYER_LUNARG_swapchain");
			validation_layers.push_back("VK_LAYER_LUNARG_device_limits");
			validation_layers.push_back("VK_LAYER_LUNARG_image");
			validation_layers.push_back("VK_LAYER_GOOGLE_unique_objects");*/
		}

		void InitDebug(VkInstance instance)
		{
			CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
			DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
			dbgBreakCallback = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");

			if (CreateDebugReportCallback == nullptr || DestroyDebugReportCallback == nullptr || dbgBreakCallback == nullptr) {
				VulkanDebug::ConsolePrint("Error fetching debug function pointers");
			}

			ErrorCheck(CreateDebugReportCallback(instance, &debugCallbackCreateInfo, nullptr, &msgCallback));
		}

		void CleanupDebugging(VkInstance instance)
		{
			DestroyDebugReportCallback(instance, msgCallback, nullptr);
			msgCallback = nullptr;
		}

		// Sets up a console window (Win32)
		void SetupConsole(std::string title)
		{
#if defined(_WIN32)
			AllocConsole();
			AttachConsole(GetCurrentProcessId());
			freopen("CON", "w", stdout);
			SetConsoleTitle(TEXT(title.c_str()));

			VulkanDebug::ConsolePrint("Debug console:");
#endif
		}

		// This is the callback that receives all debug messages from the different validation layers 
		VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
			VkDebugReportFlagsEXT       flags,
			VkDebugReportObjectTypeEXT  objectType,
			uint64_t                    object,
			size_t                      location,
			int32_t                     messageCode,
			const char*                 pLayerPrefix,
			const char*                 pMessage,
			void*                       pUserData)
		{
			std::ostringstream stream;

			stream << "VKDEBUG: ";
			if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
				stream << "INFO: ";
			}
			if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
				stream << "WARNING: ";
			}
			if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
				if(!performanceWarnings)
					return VK_FALSE;

				stream << "PERFORMANCE: ";
			}
			if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
				stream << "ERROR: ";
			}
			if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
				stream << "DEBUG: ";
			}

			stream << "@[" << pLayerPrefix << "]: ";
			stream << pMessage << std::endl;

			VulkanDebug::ConsolePrint(stream.str());

			// Critical errors will be printed in a message box (Win32)
#ifdef _WIN32
			if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
				MessageBox(nullptr, stream.str().c_str(), "Vulkan Error!", 0);
			}
#endif

			return VK_FALSE;
		}

		void ErrorCheck(VkResult result)
		{
			if (result < 0)
			{
				switch (result) {
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
					break;
				case VK_ERROR_INITIALIZATION_FAILED:
					std::cout << "VK_ERROR_INITIALIZATION_FAILED" << std::endl;
					break;
				case VK_ERROR_DEVICE_LOST:
					std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
					break;
				case VK_ERROR_MEMORY_MAP_FAILED:
					std::cout << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
					break;
				case VK_ERROR_LAYER_NOT_PRESENT:
					std::cout << "VK_ERROR_LAYER_NOT_PRESENT" << std::endl;
					break;
				case VK_ERROR_EXTENSION_NOT_PRESENT:
					std::cout << "VK_ERROR_EXTENSION_NOT_PRESENT" << std::endl;
					break;
				case VK_ERROR_FEATURE_NOT_PRESENT:
					std::cout << "VK_ERROR_FEATURE_NOT_PRESENT" << std::endl;
					break;
				case VK_ERROR_INCOMPATIBLE_DRIVER:
					std::cout << "VK_ERROR_INCOMPATIBLE_DRIVER" << std::endl;
					break;
				case VK_ERROR_TOO_MANY_OBJECTS:
					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
					break;
				case VK_ERROR_FORMAT_NOT_SUPPORTED:
					std::cout << "VK_ERROR_FORMAT_NOT_SUPPORTED" << std::endl;
					break;
				case VK_ERROR_SURFACE_LOST_KHR:
					std::cout << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;
					break;
				case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
					std::cout << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" << std::endl;
					break;
				case VK_SUBOPTIMAL_KHR:
					std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
					break;
				case VK_ERROR_OUT_OF_DATE_KHR:
					std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
					break;
				case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
					std::cout << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" << std::endl;
					break;
				case VK_ERROR_VALIDATION_FAILED_EXT:
					std::cout << "VK_ERROR_VALIDATION_FAILED_EXT" << std::endl;
					break;
				default:
					break;
				}

				assert(0 && "ErrorCheck() catched an error!");
			}
		}

		void ConsolePrint(std::string text)
		{
			std::cout << text << std::endl;
		}

		void ConsolePrint(int32_t num, std::string text)
		{
			std::cout << text << num << std::endl;
		}

		void ConsolePrint(uint32_t num, std::string text)
		{
			std::cout << text << num << std::endl;
		}

		void ConsolePrint(glm::vec3 vec, std::string text)
		{
			std::cout << text << " " << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << std::endl;
		}

		void ConsolePrint(glm::vec4 vec, std::string text)
		{
			std::cout << text << " " << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << std::endl;
		}

		void TogglePerformanceWarnings()
		{
			performanceWarnings = !performanceWarnings;
		}
	}
}	// VulkanLib namespace