#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <glm/glm.hpp>
#include "Platform.h"

#define ENUM_TO_STR(x) #x

namespace Vulkan
{
	namespace VulkanDebug
	{
		// This sets up the debug create info (so that the creation of VkInstance can use it
		// Must be called before VulkanBase::CreateInstance() 
		void SetupDebugLayers();

		// Must be called between VulkanBase::CreateInstance() and VulkanBase::CreateDevice()
		void InitDebug(VkInstance instance);

		void CleanupDebugging(VkInstance instance);
		void SetupConsole(std::string title);
		void ErrorCheck(VkResult result);

	  	void ConsolePrint(std::string text);
		void ConsolePrint(uint32_t num, std::string text = "");
		void ConsolePrint(glm::vec3 vec, std::string text = "");
		void ConsolePrint(glm::vec4 vec, std::string text = "");

		void TogglePerformanceWarnings();

		VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
			VkDebugReportFlagsEXT       flags,
			VkDebugReportObjectTypeEXT  objectType,
			uint64_t                    object,
			size_t                      location,
			int32_t                     messageCode,
			const char*                 pLayerPrefix,
			const char*                 pMessage,
			void*                       pUserData);

		// Debugging layers
		extern std::vector<const char*> validation_layers;

		// Both InitDebug() and VkInstance uses the create info from SetupDebugLayers()
		extern VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
		extern VkDebugReportCallbackEXT msgCallback;

		extern bool performanceWarnings;
	}
}	// VulkanLib namespace
