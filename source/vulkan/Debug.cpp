#include <sstream>
#include <iostream>
#include <cassert>
#include "Debug.h"
#include "utility/Timer.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Instance.h"

namespace Utopian::Vk
{
	namespace Debug
	{
		// Debug extension callbacks
		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = nullptr;
		PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = nullptr;
		PFN_vkDebugReportMessageEXT dbgBreakCallback = nullptr;

		std::vector<const char*> validation_layers;
		VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
		VkDebugReportCallbackEXT msgCallback = nullptr;
		bool performanceWarnings = true;
		std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

		void SetupDebugLayers()
		{
			// Create a console to forward standard output to
			SetupConsole("Vulkan Debug Console");

			// TODO: Add #ifdef _DEBUG
			// Configure so that Debug::VulkanDebugCallback() gets all debug messages
			debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			debugCallbackCreateInfo.pNext = nullptr;
			debugCallbackCreateInfo.flags =
				//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
				VK_DEBUG_REPORT_DEBUG_BIT_EXT |
				VK_DEBUG_REPORT_ERROR_BIT_EXT |
				VK_DEBUG_REPORT_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

			debugCallbackCreateInfo.pfnCallback = &VulkanDebugCallback;
			debugCallbackCreateInfo.pUserData = nullptr;

			// Add validation layer https://vulkan.lunarg.com/doc/sdk/1.2.135.0/windows/layer_configuration.html
			validation_layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		void InitDebug(Instance* instance)
		{
			VkInstance vkInstance = instance->GetVkHandle();
			CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
			DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
			dbgBreakCallback = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(vkInstance, "vkDebugReportMessageEXT");

			if (CreateDebugReportCallback == nullptr || DestroyDebugReportCallback == nullptr || dbgBreakCallback == nullptr) {
				Debug::ConsolePrint("Error fetching debug function pointers");
			}

			ErrorCheck(CreateDebugReportCallback(vkInstance, &debugCallbackCreateInfo, nullptr, &msgCallback));
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

			Debug::ConsolePrint(stream.str());

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
			auto currentTime = std::chrono::high_resolution_clock::now();
			auto elapsedTime = std::chrono::duration<double, std::milli>(currentTime - startTime).count();
			std::cout << (uint32_t)elapsedTime << " " << text << std::endl;
		}

		void ConsolePrint(int32_t num, std::string text)
		{
			std::cout << text << num << std::endl;
		}

		void ConsolePrint(float num, std::string text)
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

	namespace DebugMarker
	{
		bool active = false;

		PFN_vkDebugMarkerSetObjectTagEXT pfnDebugMarkerSetObjectTag = VK_NULL_HANDLE;
		PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName = VK_NULL_HANDLE;
		PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin = VK_NULL_HANDLE;
		PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd = VK_NULL_HANDLE;
		PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert = VK_NULL_HANDLE;

		void Setup(Device* device)
		{
			if (device->IsDebugMarkersEnabled())
			{
				VkDevice vkDevice = device->GetVkDevice();
				pfnDebugMarkerSetObjectTag = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetDeviceProcAddr(vkDevice, "vkDebugMarkerSetObjectTagEXT"));
				pfnDebugMarkerSetObjectName = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(vkDevice, "vkDebugMarkerSetObjectNameEXT"));
				pfnCmdDebugMarkerBegin = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(vkDevice, "vkCmdDebugMarkerBeginEXT"));
				pfnCmdDebugMarkerEnd = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(vkDevice, "vkCmdDebugMarkerEndEXT"));
				pfnCmdDebugMarkerInsert = reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(vkDevice, "vkCmdDebugMarkerInsertEXT"));

				// Set flag if at least one function pointer is present
				active = (pfnDebugMarkerSetObjectName != VK_NULL_HANDLE);
			}
		}

		void SetObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char*name)
		{
			// Check for valid function pointer (may not be present if not running in a debugging application)
			if (pfnDebugMarkerSetObjectName)
			{
				VkDebugMarkerObjectNameInfoEXT nameInfo = {};
				nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
				nameInfo.objectType = objectType;
				nameInfo.object = object;
				nameInfo.pObjectName = name;
				pfnDebugMarkerSetObjectName(device, &nameInfo);
			}
		}

		void SetObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag)
		{
			// Check for valid function pointer (may not be present if not running in a debugging application)
			if (pfnDebugMarkerSetObjectTag)
			{
				VkDebugMarkerObjectTagInfoEXT tagInfo = {};
				tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
				tagInfo.objectType = objectType;
				tagInfo.object = object;
				tagInfo.tagName = name;
				tagInfo.tagSize = tagSize;
				tagInfo.pTag = tag;
				pfnDebugMarkerSetObjectTag(device, &tagInfo);
			}
		}

		void BeginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color)
		{
			// Check for valid function pointer (may not be present if not running in a debugging application)
			if (pfnCmdDebugMarkerBegin)
			{
				VkDebugMarkerMarkerInfoEXT markerInfo = {};
				markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
				memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
				markerInfo.pMarkerName = pMarkerName;
				pfnCmdDebugMarkerBegin(cmdbuffer, &markerInfo);
			}
		}

		void Insert(VkCommandBuffer cmdbuffer, std::string markerName, glm::vec4 color)
		{
			// Check for valid function pointer (may not be present if not running in a debugging application)
			if (pfnCmdDebugMarkerInsert)
			{
				VkDebugMarkerMarkerInfoEXT markerInfo = {};
				markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
				memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
				markerInfo.pMarkerName = markerName.c_str();
				pfnCmdDebugMarkerInsert(cmdbuffer, &markerInfo);
			}
		}

		void EndRegion(VkCommandBuffer cmdBuffer)
		{
			// Check for valid function (may not be present if not running in a debugging application)
			if (pfnCmdDebugMarkerEnd)
			{
				pfnCmdDebugMarkerEnd(cmdBuffer);
			}
		}

		void SetCommandBufferName(VkDevice device, VkCommandBuffer cmdBuffer, const char* name)
		{
			SetObjectName(device, (uint64_t)cmdBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name);
		}

		void SetQueueName(VkDevice device, VkQueue queue, const char* name)
		{
			SetObjectName(device, (uint64_t)queue, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, name);
		}

		void SetImageName(VkDevice device, VkImage image, const char* name)
		{
			SetObjectName(device, (uint64_t)image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
		}

		void SetSamplerName(VkDevice device, VkSampler sampler, const char* name)
		{
			SetObjectName(device, (uint64_t)sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);
		}

		void SetBufferName(VkDevice device, VkBuffer buffer, const char* name)
		{
			SetObjectName(device, (uint64_t)buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
		}

		void SetDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char* name)
		{
			SetObjectName(device, (uint64_t)memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, name);
		}

		void SetShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char* name)
		{
			SetObjectName(device, (uint64_t)shaderModule, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
		}

		void SetPipelineName(VkDevice device, VkPipeline pipeline, const char* name)
		{
			SetObjectName(device, (uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
		}

		void SetPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char* name)
		{
			SetObjectName(device, (uint64_t)pipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name);
		}

		void SetRenderPassName(VkDevice device, VkRenderPass renderPass, const char* name)
		{
			SetObjectName(device, (uint64_t)renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name);
		}

		void SetFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char* name)
		{
			SetObjectName(device, (uint64_t)framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name);
		}

		void SetDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char* name)
		{
			SetObjectName(device, (uint64_t)descriptorSetLayout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name);
		}

		void SetDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char* name)
		{
			SetObjectName(device, (uint64_t)descriptorSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, name);
		}

		void SetSemaphoreName(VkDevice device, VkSemaphore semaphore, const char* name)
		{
			SetObjectName(device, (uint64_t)semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
		}

		void SetFenceName(VkDevice device, VkFence fence, const char* name)
		{
			SetObjectName(device, (uint64_t)fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name);
		}

		void SetEventName(VkDevice device, VkEvent _event, const char* name)
		{
			SetObjectName(device, (uint64_t)_event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, name);
		}
	};
}