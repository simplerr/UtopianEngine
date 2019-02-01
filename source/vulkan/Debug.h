#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <glm/glm.hpp>
#include <chrono>
#include "vulkan/VulkanInclude.h"
#include "utility/Platform.h"

#define ENUM_TO_STR(x) #x

namespace Utopian::Vk
{
	namespace Debug
	{
		// This sets up the debug create info (so that the creation of VkInstance can use it
		// Must be called before VulkanBase::CreateInstance() 
		void SetupDebugLayers();

		// Must be called between VulkanBase::CreateInstance() and VulkanBase::CreateDevice()
		void InitDebug(Instance* instance);

		void CleanupDebugging(VkInstance instance);
		void SetupConsole(std::string title);
		void ErrorCheck(VkResult result);

		/** Prints timestamped text to the console. */
	  	void ConsolePrint(std::string text);
		 
		void ConsolePrint(uint32_t num, std::string text = "");
		void ConsolePrint(int32_t num, std::string text = "");
		void ConsolePrint(float num, std::string text = "");
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
		extern std::chrono::high_resolution_clock::time_point startTime;
	}

	/**
	 * Setup and functions for the VK_EXT_debug_marker_extension
	 * Extension spec can be found at https://github.com/KhronosGroup/Vulkan-Docs/blob/1.0-VK_EXT_debug_marker/doc/specs/vulkan/appendices/VK_EXT_debug_marker.txt
	 * Note that the extension will only be present if run from an offline debugging application
	 * The actual check for extension presence and enabling it on the device is done in Device
	 * Implementation from https://github.com/SaschaWillems/Vulkan, guide: https://www.saschawillems.de/?page_id=2017
	 */
	namespace DebugMarker
	{
		/** Set to true if function pointer for the debug marker are available. */
		extern bool active;

		/** Get function pointers for the debug report extensions from the device. */
		void Setup(Device* device);

		/**
		 * Sets the debug name of an object.
		 * All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
		 * along with the object type. 
		 */
		void SetObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char*name);

		/** Set the tag for an object. */
		void SetObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag);

		/** Start a new debug marker region. */
		void BeginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color);

		/** Insert a new debug marker into the command buffer. */
		void Insert(VkCommandBuffer cmdbuffer, std::string markerName, glm::vec4 color);

		/** End the current debug marker region. */
		void EndRegion(VkCommandBuffer cmdBuffer);

		/** Object specific naming functions. */
		void SetCommandBufferName(VkDevice device, VkCommandBuffer cmdBuffer, const char* name);
		void SetQueueName(VkDevice device, VkQueue queue, const char* name);
		void SetImageName(VkDevice device, VkImage image, const char* name);
		void SetSamplerName(VkDevice device, VkSampler sampler, const char* name);
		void SetBufferName(VkDevice device, VkBuffer buffer, const char* name);
		void SetDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char* name);
		void SetShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char* name);
		void SetPipelineName(VkDevice device, VkPipeline pipeline, const char* name);
		void SetPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char* name);
		void SetRenderPassName(VkDevice device, VkRenderPass renderPass, const char* name);
		void SetFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char* name);
		void SetDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char* name);
		void SetDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char* name);
		void SetSemaphoreName(VkDevice device, VkSemaphore semaphore, const char* name);
		void SetFenceName(VkDevice device, VkFence fence, const char* name);
		void SetEventName(VkDevice device, VkEvent _event, const char* name);
	};
}	// VulkanLib namespace
