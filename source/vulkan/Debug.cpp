#include <sstream>
#include <iostream>
#include <cassert>
#include "Debug.h"
#include "utility/Timer.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Instance.h"
#include "vulkan/Debug.h"
#include "core/Log.h"

namespace Utopian::Vk
{
	namespace Debug
	{
		std::vector<const char*> validation_layers;
		std::vector<VkValidationFeatureEnableEXT> enabledValidationFeatures;
		VkValidationFeaturesEXT validationFeatures = {};
		bool performanceWarnings = true;
		std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = {};
		VkDebugUtilsMessengerEXT debugUtilsMessenger = nullptr;

      std::function<void(std::string)> mUserLogCallback = nullptr;

		void SetupDebugLayers()
		{
			// Debug utils setup
			debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
												   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
												   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
												   VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

			debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
											   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
											   //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
			debugUtilsCreateInfo.pfnUserCallback = VulkanDebugMessengerCallback;

			// Validation features setup
			enabledValidationFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);
#ifdef ENABLE_GPU_VALIDATION
			enabledValidationFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
#endif
			validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
			validationFeatures.enabledValidationFeatureCount = enabledValidationFeatures.size();
			validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures.data();
			debugUtilsCreateInfo.pNext = &validationFeatures;

			// Add validation layer https://vulkan.lunarg.com/doc/sdk/1.2.135.0/windows/layer_configuration.html
			validation_layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		void InitDebug(Instance* instance)
		{
			PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->GetVkHandle(), "vkCreateDebugUtilsMessengerEXT");

			if (CreateDebugUtilsMessenger == nullptr)
				UTO_LOG("Error fetching vkCreateDebugUtilsMessengerEXT function pointer");
			else
				ErrorCheck(CreateDebugUtilsMessenger(instance->GetVkHandle(), &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger));
		}

		void CleanupDebugging(VkInstance instance)
		{
			PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

			if (DestroyDebugUtilsMessenger != nullptr)
				DestroyDebugUtilsMessenger(instance, debugUtilsMessenger, nullptr);
		}


		VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugMessengerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData)
		{
			std::ostringstream stream;

			std::string debugPrefix = "Unknown";
			if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
				debugPrefix = "Verbose: ";
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
				debugPrefix = "Info: ";
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				debugPrefix = "Warning: ";
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
				debugPrefix = "Error: ";

			stream << std::endl << "-------------------------" << std::endl;
			stream << callbackData->messageIdNumber << ":" << callbackData->pMessage << std::endl;
			// callbackData->pMessageIdName is a nullptr when RenderDoc is printing information causing the application to crash

			if (callbackData->objectCount > 0)
			{
				for (uint32_t object = 0; object < callbackData->objectCount; object++)
				{
					std::string name = "Unnamed";
					if (callbackData->pObjects[object].pObjectName != nullptr)
						name = callbackData->pObjects[object].pObjectName;

					stream << "Object[" << object << "] - Type: " << ObjectTypeToString(callbackData->pObjects[object].objectType);
					stream << ", Value: " << (void*)(callbackData->pObjects[object].objectHandle);
					stream << ", Name: " << name;
					stream << std::endl;
				}
			}

			if (callbackData->cmdBufLabelCount > 0)
			{
				stream << "Command Buffer Labels - " << callbackData->cmdBufLabelCount;
				for (uint32_t label = 0; label < callbackData->cmdBufLabelCount; label++)
				{
					std::string name = "Unnamed";
					if (callbackData->pCmdBufLabels[label].pLabelName != nullptr)
						name = callbackData->pCmdBufLabels[label].pLabelName;

					stream << "Command Buffer Label[" << label << "]: " << name << std::endl;
			 	}
			}

			stream << "-------------------------" << std::endl;

			UTO_LOG(stream.str());

#ifdef _WIN32
			// Critical errors will be printed in a message box
			if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
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
               UTO_LOG("VK_ERROR_OUT_OF_HOST_MEMORY");
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
               UTO_LOG("VK_ERROR_OUT_OF_DEVICE_MEMORY");
					break;
				case VK_ERROR_INITIALIZATION_FAILED:
               UTO_LOG("VK_ERROR_INITIALIZATION_FAILED");
					break;
				case VK_ERROR_DEVICE_LOST:
               UTO_LOG("VK_ERROR_DEVICE_LOST");
					break;
				case VK_ERROR_MEMORY_MAP_FAILED:
               UTO_LOG("VK_ERROR_MEMORY_MAP_FAILED");
					break;
				case VK_ERROR_LAYER_NOT_PRESENT:
               UTO_LOG("VK_ERROR_LAYER_NOT_PRESENT");
					break;
				case VK_ERROR_EXTENSION_NOT_PRESENT:
               UTO_LOG("VK_ERROR_EXTENSION_NOT_PRESENT");
					break;
				case VK_ERROR_FEATURE_NOT_PRESENT:
               UTO_LOG("VK_ERROR_FEATURE_NOT_PRESENT");
					break;
				case VK_ERROR_INCOMPATIBLE_DRIVER:
               UTO_LOG("VK_ERROR_INCOMPATIBLE_DRIVER");
					break;
				case VK_ERROR_TOO_MANY_OBJECTS:
               UTO_LOG("VK_ERROR_TOO_MANY_OBJECTS");
					break;
				case VK_ERROR_FORMAT_NOT_SUPPORTED:
               UTO_LOG("VK_ERROR_FORMAT_NOT_SUPPORTED");
					break;
				case VK_ERROR_SURFACE_LOST_KHR:
               UTO_LOG("VK_ERROR_SURFACE_LOST_KHR");
					break;
				case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
               UTO_LOG("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
					break;
				case VK_SUBOPTIMAL_KHR:
               UTO_LOG("VK_SUBOPTIMAL_KHR");
					break;
				case VK_ERROR_OUT_OF_DATE_KHR:
               UTO_LOG("VK_ERROR_OUT_OF_DATE_KHR");
					break;
				case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
               UTO_LOG("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR");
					break;
				case VK_ERROR_VALIDATION_FAILED_EXT:
               UTO_LOG("VK_ERROR_VALIDATION_FAILED_EXT");
					break;
				default:
					break;
				}

			assert(0 && "ErrorCheck() catched an error!");
			}
		}
		
		std::string ObjectTypeToString(const VkObjectType objectType)
		{
			switch (objectType)
			{
			case VK_OBJECT_TYPE_INSTANCE:
				return "VkInstance";
			case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
				return "VkPhysicalDevice";
			case VK_OBJECT_TYPE_DEVICE:
				return "VkDevice";
			case VK_OBJECT_TYPE_QUEUE:
				return "VkQueue";
			case VK_OBJECT_TYPE_SEMAPHORE:
				return "VkSemaphore";
			case VK_OBJECT_TYPE_COMMAND_BUFFER:
				return "VkCommandBuffer";
			case VK_OBJECT_TYPE_FENCE:
				return "VkFence";
			case VK_OBJECT_TYPE_DEVICE_MEMORY:
				return "VkDeviceMemory";
			case VK_OBJECT_TYPE_BUFFER:
				return "VkBuffer";
			case VK_OBJECT_TYPE_IMAGE:
				return "VkImage";
			case VK_OBJECT_TYPE_EVENT:
				return "VkEvent";
			case VK_OBJECT_TYPE_QUERY_POOL:
				return "VkQueryPool";
			case VK_OBJECT_TYPE_BUFFER_VIEW:
				return "VkBufferView";
			case VK_OBJECT_TYPE_IMAGE_VIEW:
				return "VkImageView";
			case VK_OBJECT_TYPE_SHADER_MODULE:
				return "VkShaderModule";
			case VK_OBJECT_TYPE_PIPELINE_CACHE:
				return "VkPipelineCache";
			case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
				return "VkPipelineLayout";
			case VK_OBJECT_TYPE_RENDER_PASS:
				return "VkRenderPass";
			case VK_OBJECT_TYPE_PIPELINE:
				return "VkPipeline";
			case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
				return "VkDescriptorSetLayout";
			case VK_OBJECT_TYPE_SAMPLER:
				return "VkSampler";
			case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
				return "VkDescriptorPool";
			case VK_OBJECT_TYPE_DESCRIPTOR_SET:
				return "VkDescriptorSet";
			case VK_OBJECT_TYPE_FRAMEBUFFER:
				return "VkFramebuffer";
			case VK_OBJECT_TYPE_COMMAND_POOL:
				return "VkCommandPool";
			case VK_OBJECT_TYPE_SURFACE_KHR:
				return "VkSurfaceKHR";
			case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
				return "VkSwapchainKHR";
			case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
				return "VkDebugReportCallbackEXT";
			case VK_OBJECT_TYPE_DISPLAY_KHR:
				return "VkDisplayKHR";
			case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
				return "VkDisplayModeKHR";
			case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR:
				return "VkDescriptorUpdateTemplateKHR";
			default:
				return "Unknown Type";
    		}
		}

		void TogglePerformanceWarnings()
		{
			performanceWarnings = !performanceWarnings;
		}
	}

	namespace DebugLabel
	{
		bool active = false;

		PFN_vkCmdBeginDebugUtilsLabelEXT pfnBeginDebugUtilsLabel = VK_NULL_HANDLE;
		PFN_vkCmdEndDebugUtilsLabelEXT pfnEndDebugUtilsLabel = VK_NULL_HANDLE;
		PFN_vkCmdInsertDebugUtilsLabelEXT pfnInsertDebugUtilsLabel = VK_NULL_HANDLE;
		PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectName = VK_NULL_HANDLE;
		PFN_vkSetDebugUtilsObjectTagEXT pfnSetDebugUtilsObjectTag = VK_NULL_HANDLE;

		void Setup(Device* device)
		{
			//if (device->IsDebugMarkersEnabled())
			{
				VkDevice vkDevice = device->GetVkDevice();
				pfnBeginDebugUtilsLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(vkDevice, "vkCmdBeginDebugUtilsLabelEXT"));;
				pfnEndDebugUtilsLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(vkDevice, "vkCmdEndDebugUtilsLabelEXT"));;
				pfnInsertDebugUtilsLabel = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(vkGetDeviceProcAddr(vkDevice, "vkCmdInsertDebugUtilsLabelEXT"));;
				pfnSetDebugUtilsObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(vkDevice, "vkSetDebugUtilsObjectNameEXT"));;
				pfnSetDebugUtilsObjectTag = reinterpret_cast<PFN_vkSetDebugUtilsObjectTagEXT>(vkGetDeviceProcAddr(vkDevice, "vkSetDebugUtilsObjectTagEXT"));;
			
				if (pfnBeginDebugUtilsLabel == nullptr)
					UTO_LOG("Error fetching vkCmdBeginDebugUtilsLabelEXT function pointer");

				// Set flag if at least one function pointer is present
				active = (pfnBeginDebugUtilsLabel != VK_NULL_HANDLE);
			}
		}

		void BeginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color)
		{
			if (pfnBeginDebugUtilsLabel)
			{
				VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
				label.pLabelName = pMarkerName;
				memcpy(label.color, &color[0], sizeof(float) * 4);
				pfnBeginDebugUtilsLabel(cmdbuffer, &label);
			}
		}

		void EndRegion(VkCommandBuffer cmdBuffer)
		{
			// Check for valid function (may not be present if not running in a debugging application)
			if (pfnEndDebugUtilsLabel)
			{
				pfnEndDebugUtilsLabel(cmdBuffer);
			}
		}

		void Insert(VkCommandBuffer cmdbuffer, std::string markerName, glm::vec4 color)
		{
			// Check for valid function pointer (may not be present if not running in a debugging application)
			if (pfnInsertDebugUtilsLabel)
			{
				VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
				label.pLabelName = markerName.c_str();
				memcpy(label.color, &color[0], sizeof(float) * 4);
				pfnInsertDebugUtilsLabel(cmdbuffer, &label);
			}
		}

		void SetObjectName(VkDevice device, uint64_t object, VkObjectType objectType, const char* name)
		{
			// Check for valid function pointer (may not be present if not running in a debugging application)
			if (pfnSetDebugUtilsObjectName)
			{
				VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
				name_info.objectType = objectType;
				name_info.objectHandle = object;
				name_info.pObjectName = name;
				pfnSetDebugUtilsObjectName(device, &name_info);
			}
		}

		void SetObjectTag(VkDevice device, uint64_t object, VkObjectType objectType, uint64_t name, size_t tagSize, const void* tag)
		{
			// Check for valid function pointer (may not be present if not running in a debugging application)
			if (pfnSetDebugUtilsObjectTag)
			{
				VkDebugUtilsObjectTagInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT };
				info.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
				info.objectHandle = object;
				info.tagName = name;
				info.tagSize = tagSize;
				info.pTag = tag;
				pfnSetDebugUtilsObjectTag(device, &info);
			}
		}

		void SetCommandBufferName(VkDevice device, VkCommandBuffer cmdBuffer, const char* name)
		{
			SetObjectName(device, (uint64_t)cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
		}

		void SetQueueName(VkDevice device, VkQueue queue, const char* name)
		{
			SetObjectName(device, (uint64_t)queue, VK_OBJECT_TYPE_QUEUE, name);
		}

		void SetImageName(VkDevice device, VkImage image, const char* name)
		{
			SetObjectName(device, (uint64_t)image, VK_OBJECT_TYPE_IMAGE, name);
		}

		void SetSamplerName(VkDevice device, VkSampler sampler, const char* name)
		{
			SetObjectName(device, (uint64_t)sampler, VK_OBJECT_TYPE_SAMPLER, name);
		}

		void SetBufferName(VkDevice device, VkBuffer buffer, const char* name)
		{
			SetObjectName(device, (uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, name);
		}

		void SetDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char* name)
		{
			SetObjectName(device, (uint64_t)memory, VK_OBJECT_TYPE_DEVICE_MEMORY, name);
		}

		void SetShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char* name)
		{
			SetObjectName(device, (uint64_t)shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, name);
		}

		void SetPipelineName(VkDevice device, VkPipeline pipeline, const char* name)
		{
			SetObjectName(device, (uint64_t)pipeline, VK_OBJECT_TYPE_PIPELINE, name);
		}

		void SetPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char* name)
		{
			SetObjectName(device, (uint64_t)pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, name);
		}

		void SetRenderPassName(VkDevice device, VkRenderPass renderPass, const char* name)
		{
			SetObjectName(device, (uint64_t)renderPass, VK_OBJECT_TYPE_RENDER_PASS, name);
		}

		void SetFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char* name)
		{
			SetObjectName(device, (uint64_t)framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, name);
		}

		void SetDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char* name)
		{
			SetObjectName(device, (uint64_t)descriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, name);
		}

		void SetDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char* name)
		{
			SetObjectName(device, (uint64_t)descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, name);
		}

		void SetSemaphoreName(VkDevice device, VkSemaphore semaphore, const char* name)
		{
			SetObjectName(device, (uint64_t)semaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
		}

		void SetFenceName(VkDevice device, VkFence fence, const char* name)
		{
			SetObjectName(device, (uint64_t)fence, VK_OBJECT_TYPE_FENCE, name);
		}

		void SetEventName(VkDevice device, VkEvent _event, const char* name)
		{
			SetObjectName(device, (uint64_t)_event, VK_OBJECT_TYPE_EVENT, name);
		}
	};
}