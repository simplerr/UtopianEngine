#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <glm/glm.hpp>
#include <chrono>
#include <functional>
#include <string>
#include "vulkan/VulkanPrerequisites.h"
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
      void ErrorCheck(VkResult result);
       
      void TogglePerformanceWarnings();

      VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugMessengerCallback(
         VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
         VkDebugUtilsMessageTypeFlagsEXT messageType,
         const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
         void* userData);

      std::string ObjectTypeToString(const VkObjectType objectType);

      // Debugging layers
      extern std::vector<const char*> validation_layers;

      // Both InitDebug() and VkInstance uses the create info from SetupDebugLayers()
      extern VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
      extern VkDebugUtilsMessengerEXT debugUtilsMessenger;

      extern VkValidationFeaturesEXT validationFeatures;
      extern std::vector<VkValidationFeatureEnableEXT> enabledValidationFeatures;

      extern bool performanceWarnings;
   }

   /**
    * References:
    * https://github.com/KhronosGroup/Vulkan-Samples/blob/master/samples/extensions/debug_utils/debug_utils_tutorial.md
    * https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf
    */
   namespace DebugLabel
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
      void SetObjectName(VkDevice device, uint64_t object, VkObjectType objectType, const char* name);

      /** Set the tag for an object. */
      void SetObjectTag(VkDevice device, uint64_t object, VkObjectType objectType, uint64_t name, size_t tagSize, const void* tag);

      /** Start a new debug label region. */
      void BeginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color);

      /** Insert a new debug label into the command buffer. */
      void Insert(VkCommandBuffer cmdbuffer, std::string markerName, glm::vec4 color);

      /** End the current debug label region. */
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
}
