#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "vulkan/VulkanPrerequisites.h"
#include "utopian/utility/Common.h"
#include "../external/vk_mem_alloc.h"

namespace Utopian::Vk
{
   struct VulkanVersion
   {
      VulkanVersion();
      VulkanVersion(uint32_t apiVersion);

      uint32_t major;
      uint32_t minor;
      uint32_t patch;
      std::string version;
   };

   /** Wrapper for the Vulkan device. */
   class Device
   {
   public:
      Device(Instance* instance, bool enableValidation = false);
      ~Device();

      /**
       * Returns the graphics queue associated with the device.
       *
       * @note Currently only one queue is fetched from the device.
       */
      Queue* GetQueue() const;

      /** Adds the buffers the a garbage collect list that will be destroyed once no command buffer is active. */
      void QueueDestroy(SharedPtr<Vk::Buffer>& buffer);
      void QueueDestroy(VkPipeline pipeline);
      void QueueDestroy(SharedPtr<Vk::Image> image);
      void QueueDestroy(SharedPtr<Vk::Sampler> sampler);

      /** Destroys all Vulkan resources that have been added to the garbage collect list. */
      void GarbageCollect();

      /** Queues an update of a descriptor set that is performed once no command buffer is active. */
      void QueueDescriptorUpdate(Vk::DescriptorSet* descriptorSet);

      /** Perform descriptor set update on queued objects. */
      void UpdateDescriptorSets();

      /* Memory management. */
      VmaAllocation AllocateMemory(Image* image, VkMemoryPropertyFlags flags);
      VmaAllocation AllocateMemory(Buffer* buffer, VkMemoryPropertyFlags flags);
      void MapMemory(VmaAllocation allocation, void** data);
      void UnmapMemory(VmaAllocation allocation);
      void FreeMemory(VmaAllocation allocation);
      void GetAllocationInfo(VmaAllocation allocation, VkDeviceMemory& memory, VkDeviceSize& offset);

      /** Returns the combined memory budget for all heaps matching heapFlags. */
      VmaBudget GetMemoryBudget(VkMemoryHeapFlags heapFlags);

      /** Returns detailed memory statistics. */
      void GetMemoryStats(VmaStats* stats);

      /** Writes memory statistics to .json file, can be visualized with VmpaDumpVis.py. */
      void DumpMemoryStats(std::string filename);

      /* Returns device memory properties. */
      const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const;

      /* Returns physical device properties. */
      const VkPhysicalDeviceProperties& GetProperties() const;

      /** Returns the command pool from the device which new command buffers can be allocated from. */
      CommandPool* GetCommandPool() const;

      VkPhysicalDevice GetPhysicalDevice() const;
      VkDevice GetVkDevice() const;
      uint32_t GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex) const;
      bool IsDebugMarkersEnabled() const;
      uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const;
      VulkanVersion GetVulkanVersion() const;

   private:
      void RetrievePhysical(Instance* instance);
      void RetrieveQueueFamilyProperites();
      void CreateLogical(bool enableValidation);
      void RetrieveSupportedExtensions();
      bool IsExtensionSupported(std::string extension);

   private:
      VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
      VkDevice mDevice = VK_NULL_HANDLE;
      VkPhysicalDeviceProperties mPhysicalDeviceProperties;
      VkPhysicalDeviceMemoryProperties mDeviceMemoryProperties;
      VkPhysicalDeviceFeatures mEnabledFeatures {};
      VkPhysicalDeviceFeatures mAvailableFeatures;
      std::vector<std::string> mSupportedExtensions;
      std::vector<VkQueueFamilyProperties> mQueueFamilyProperties;
      VulkanVersion mVulkanVersion;
      VmaAllocator mAllocator;

      CommandPool* mCommandPool = nullptr;
      Queue* mQueue = nullptr;
      bool mDebugMarkersEnabled = false;

      // Garbage collection
      std::vector<SharedPtr<Vk::Buffer>> mBuffersToFree;
      std::vector<VkPipeline> mPipelinesToFree;
      std::vector<SharedPtr<Vk::Image>> mImagesToFree;
      std::vector<SharedPtr<Vk::Sampler>> mSamplersToFree;

      // Descriptor update
      std::vector<Vk::DescriptorSet*> mDescriptorSetUpdateQueue;
   };
}
