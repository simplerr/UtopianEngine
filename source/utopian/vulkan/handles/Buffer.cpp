#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/Debug.h"
#include <vulkan/vulkan_core.h>

namespace Utopian::Vk
{
   Buffer::Buffer(Device* device, std::string debugName)
      : Handle(device, nullptr)
   {
      SetDebugName(debugName);
   }

   Buffer::Buffer(const BUFFER_CREATE_INFO& createInfo, Device* device)
      : Handle(device, nullptr)
   {
      SetDebugName(createInfo.name);
      Create(device, createInfo.usageFlags, createInfo.memoryPropertyFlags, createInfo.size, createInfo.data);
   }
   
   Buffer::~Buffer()
   {
      Destroy();
   }

   void Buffer::Create(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
   {
      mDevice = device;
      mSize = (uint32_t)size;

      VkMemoryAllocateInfo memAllocInfo = {};
      memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memAllocInfo.pNext = NULL;
      memAllocInfo.allocationSize = 0;
      memAllocInfo.memoryTypeIndex = 0;

      VkBufferCreateInfo bufferCreateInfo = {};
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.pNext = NULL;
      bufferCreateInfo.usage = usageFlags;
      bufferCreateInfo.size = size;
      bufferCreateInfo.flags = 0;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      VkDevice vkDevice = mDevice->GetVkDevice();
      Debug::ErrorCheck(vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &mHandle));

      mAllocation = device->AllocateMemory(this, memoryPropertyFlags);

      if (data != nullptr)
      {
         UpdateMemory(data, size);
      }

      DebugLabel::SetBufferName(GetVkDevice(), mHandle, GetDebugName().c_str());
   }

   void Buffer::Destroy()
   {
      if (mHandle != VK_NULL_HANDLE)
      {
         if (mMapped)
            mDevice->UnmapMemory(mAllocation);

         vkDestroyBuffer(mDevice->GetVkDevice(), mHandle, nullptr);
         mDevice->FreeMemory(mAllocation);

         mHandle = VK_NULL_HANDLE;
      }
   }

   void Buffer::UpdateMemory(void* data, VkDeviceSize size)
   {
      void *mapped;
      MapMemory(&mapped);
      memcpy(mapped, data, size);
      UnmapMemory();
   }

   void Buffer::MapMemory(void** data)
   {
      mDevice->MapMemory(mAllocation, data);
      mMapped = true;
   }

   void Buffer::UnmapMemory()
   {
      if (mMapped)
         mDevice->UnmapMemory(mAllocation);

      mMapped = false;
   }

   void Buffer::Copy(CommandBuffer* commandBuffer, Buffer* destination)
   {
      VkBufferCopy regions;
      regions.dstOffset = 0;
      regions.srcOffset = 0;
      regions.size = GetSize();
      vkCmdCopyBuffer(commandBuffer->GetVkHandle(), GetVkHandle(), destination->GetVkHandle(), 1, &regions);
   }

   void Buffer::Copy(CommandBuffer* commandBuffer, Image* destination, const std::vector<VkBufferImageCopy>& regions)
   {
      vkCmdCopyBufferToImage(
         commandBuffer->GetVkHandle(),
         GetVkHandle(),
         destination->GetVkHandle(),
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         static_cast<uint32_t>(regions.size()),
         regions.data()
      );
   }

   uint32_t Buffer::GetSize() const
   {
      return mSize;
   }
}