#include <stdexcept>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/Debug.h"
#include "Image.h"

namespace Utopian::Vk
{
   Image::Image(const IMAGE_CREATE_INFO& createInfo, Device* device)
      : Handle(device, nullptr)
   {
      CreateInternal(createInfo, device); 
   }

   Image::Image(Device* device)
      : Handle(device, nullptr)
   {
   }

   Image::~Image()
   {
      // Image handles all the cleanup itself, instead of Handle
      vkDestroyImageView(GetVkDevice(), mImageView, nullptr);
      vkDestroyImage(GetVkDevice(), mHandle, nullptr);

      for (auto& imageView : mLayerViews)
         vkDestroyImageView(GetVkDevice(), imageView, nullptr);

      GetDevice()->FreeMemory(mAllocation);
   }

   void Image::CreateInternal(const IMAGE_CREATE_INFO& createInfo, Device* device)
   {
      mWidth = createInfo.width;
      mHeight = createInfo.height;
      mDepth = createInfo.depth;
      mFormat = createInfo.format;
      mFinalImageLayout = createInfo.finalImageLayout;
      mNumMipLevels = createInfo.mipLevels;
      mLayerCount = createInfo.arrayLayers;
      mCurrentLayout = createInfo.initialLayout;
      SetDebugName(createInfo.name);

      VkImageCreateInfo imageCreateInfo = {};
      imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageCreateInfo.format = createInfo.format;
      imageCreateInfo.imageType = (createInfo.depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D);
      imageCreateInfo.extent = { createInfo.width, createInfo.height, createInfo.depth };
      imageCreateInfo.mipLevels = createInfo.mipLevels;
      imageCreateInfo.arrayLayers = createInfo.arrayLayers;
      imageCreateInfo.tiling = createInfo.tiling;
      imageCreateInfo.usage = createInfo.usage;
      imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageCreateInfo.initialLayout = mCurrentLayout;
      imageCreateInfo.flags = createInfo.flags;

      CreateImage(imageCreateInfo, createInfo.properties);

      DebugLabel::SetImageName(GetVkDevice(), mHandle, GetDebugName().c_str());

      // Connect the view with the image
      VkImageViewCreateInfo viewCreateInfo = {};
      viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

      if (createInfo.depth == 1)
         viewCreateInfo.viewType = (createInfo.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY);
      else if (createInfo.depth > 1)
         viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;

      viewCreateInfo.format = createInfo.format;
      viewCreateInfo.subresourceRange = {};
      viewCreateInfo.subresourceRange.aspectMask = createInfo.aspectFlags;
      viewCreateInfo.subresourceRange.baseMipLevel = 0;
      viewCreateInfo.subresourceRange.levelCount = createInfo.mipLevels;
      viewCreateInfo.subresourceRange.baseArrayLayer = 0;
      viewCreateInfo.subresourceRange.layerCount = createInfo.arrayLayers;

      if (createInfo.flags == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
         viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

      CreateView(viewCreateInfo);

      // If multiple array layers create one image view per layer
      if (createInfo.arrayLayers > 1)
      {
         for (uint32_t layer = 0; layer < createInfo.arrayLayers; layer++)
         {
            viewCreateInfo = {};
            viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            viewCreateInfo.format = createInfo.format;
            viewCreateInfo.subresourceRange = {};
            viewCreateInfo.subresourceRange.aspectMask = createInfo.aspectFlags;
            viewCreateInfo.subresourceRange.baseMipLevel = 0;
            viewCreateInfo.subresourceRange.levelCount = createInfo.mipLevels;
            viewCreateInfo.subresourceRange.baseArrayLayer = layer;
            viewCreateInfo.subresourceRange.layerCount = 1;
            viewCreateInfo.image = mHandle;

            VkImageView layerView = VK_NULL_HANDLE;
            Debug::ErrorCheck(vkCreateImageView(GetVkDevice(), &viewCreateInfo, nullptr, &layerView));
            mLayerViews.push_back(layerView);
         }
      }

      if (createInfo.transitionToFinalLayout)
      {
         Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
         LayoutTransition(commandBuffer, GetFinalLayout());
         commandBuffer.Flush();
      }
   }

   void Image::CreateImage(VkImageCreateInfo imageCreateInfo, VkMemoryPropertyFlags properties)
   {
      Debug::ErrorCheck(vkCreateImage(GetVkDevice(), &imageCreateInfo, nullptr, &mHandle));

      mAllocation = GetDevice()->AllocateMemory(this, properties);
   }

   void Image::CreateView(VkImageViewCreateInfo viewCreateInfo)
   {
      viewCreateInfo.image = mHandle;
      Debug::ErrorCheck(vkCreateImageView(GetVkDevice(), &viewCreateInfo, nullptr, &mImageView));
   }

   void Image::LayoutTransition(const CommandBuffer& commandBuffer, VkImageLayout newLayout)
   {
      VkImageSubresourceRange subresourceRange = {};
      subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      subresourceRange.baseMipLevel = 0;
      subresourceRange.levelCount = mNumMipLevels;
      subresourceRange.layerCount = mLayerCount;

      VkImageMemoryBarrier barrier = {};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.oldLayout = mCurrentLayout;
      barrier.newLayout = newLayout;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.image = GetVkHandle();
      barrier.subresourceRange = subresourceRange;

      VkPipelineStageFlags srcStageMask;
      VkPipelineStageFlags dstStageMask;

      if (mCurrentLayout == VK_IMAGE_LAYOUT_PREINITIALIZED ||
         mCurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED)
      {
         barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
         srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
      }
      else if (mCurrentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
      {
         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
         srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
      }
      else if (mCurrentLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
      {
         barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
         srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
      }
      else if (mCurrentLayout == VK_IMAGE_LAYOUT_GENERAL)
      {
         barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
         srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
      }
      else if (mCurrentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
      {
         barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
         srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
      }
      else
      {
         throw std::invalid_argument("Unsupported layout transition!");
      }

      if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
      {
         barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
         dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
      }
      else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
      {
         barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
         dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
      }
      else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
             newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
      {
         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
         dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      }
      else if (newLayout == VK_IMAGE_LAYOUT_GENERAL)
      {
         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
         dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      }
      else
      {
         throw std::invalid_argument("Unsupported layout transition!");
      }

      vkCmdPipelineBarrier(
         commandBuffer.GetVkHandle(),
         srcStageMask, dstStageMask,
         0,
         0, nullptr,
         0, nullptr,
         1, &barrier
      );

      mCurrentLayout = newLayout;
   }

   void Image::Copy(const CommandBuffer& commandBuffer, Image* destination)
   {
      VkImageSubresourceLayers subResource = {};
      subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      subResource.baseArrayLayer = 0;
      subResource.mipLevel = 0;
      subResource.layerCount = 1;

      VkImageCopy region = {};
      region.srcSubresource = subResource;
      region.dstSubresource = subResource;
      region.srcOffset = { 0, 0, 0 };
      region.dstOffset = { 0, 0, 0 };
      region.extent.width = mWidth;
      region.extent.height = mHeight;
      region.extent.depth = mDepth;

      vkCmdCopyImage(
         commandBuffer.GetVkHandle(),
         GetVkHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
         destination->GetVkHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         1, &region
      );
   }

   void Image::Blit(const CommandBuffer& commandBuffer, Image* destination)
   {
      // Blit
      VkOffset3D srcBlitSize;
      srcBlitSize.x = GetWidth();
      srcBlitSize.y = GetHeight();
      srcBlitSize.z = 1;
      VkOffset3D dstBlitSize;
      dstBlitSize.x = destination->GetWidth();
      dstBlitSize.y = destination->GetHeight();
      dstBlitSize.z = 1;
      VkImageBlit imageBlitRegion{};
      imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imageBlitRegion.srcSubresource.layerCount = 1;
      imageBlitRegion.srcOffsets[1] = srcBlitSize;
      imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imageBlitRegion.dstSubresource.layerCount = 1;
      imageBlitRegion.dstOffsets[1] = dstBlitSize;

      vkCmdBlitImage(
         commandBuffer.GetVkHandle(),
         GetVkHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
         destination->GetVkHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         1,
         &imageBlitRegion,
         VK_FILTER_NEAREST);
   }

   void Image::SetFinalLayout(VkImageLayout finalLayout)
   {
      mFinalImageLayout = finalLayout;
   }

   void Image::UpdateMemory(void* data, VkDeviceSize size)
   {
      assert(data);

      void *mapped;
      MapMemory(&mapped);
      memcpy(mapped, data, size);
      UnmapMemory();
   }

   void Image::MapMemory(void** data)
   {
      GetDevice()->MapMemory(mAllocation, data);
   }

   void Image::UnmapMemory()
   {
      GetDevice()->UnmapMemory(mAllocation);
   }

   VkImageView Image::GetView() const
   {
      return mImageView;
   }

   VkImageView Image::GetLayerView(uint32_t layer) const
   {
      if (layer < mLayerViews.size())
      {
         return mLayerViews[layer];
      }

      assert(0);
      return VK_NULL_HANDLE;
   }

   VkFormat Image::GetFormat() const
   {
      return mFormat;
   }

   VkImageLayout Image::GetFinalLayout() const
   {
      return mFinalImageLayout;
   }

   uint32_t Image::GetWidth() const
   {
      return mWidth;
   }

   uint32_t Image::GetHeight() const
   {
      return mHeight;
   }

   VkSubresourceLayout Image::GetSubresourceLayout(Device* device) const
   {
      VkImageSubresource subresource = {};
      subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

      VkSubresourceLayout subresourceLayout;
      vkGetImageSubresourceLayout(GetVkDevice(), GetVkHandle(), &subresource, &subresourceLayout);

      return subresourceLayout;
   }

   ImageColor::ImageColor(Device* device, uint32_t width, uint32_t height, VkFormat format, std::string debugName, uint32_t arrayLayers)
      : Image(device)
   {
      IMAGE_CREATE_INFO createInfo;
      createInfo.width = width;
      createInfo.height = height;
      createInfo.format = format;
      createInfo.arrayLayers = arrayLayers;
      createInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
      createInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.name = debugName;
      CreateInternal(createInfo, device);
   }

   ImageDepth::ImageDepth(Device* device, uint32_t width, uint32_t height, VkFormat format, std::string debugName, uint32_t arrayLayers)
      : Image(device)
   {
      IMAGE_CREATE_INFO createInfo;
      createInfo.width = width;
      createInfo.height = height;
      createInfo.format = format;
      createInfo.arrayLayers = arrayLayers;
      createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
      createInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
      createInfo.name = debugName;
      CreateInternal(createInfo, device);
   }

   ImageStorage::ImageStorage(Device* device, uint32_t width, uint32_t height, uint32_t depth, std::string debugName, VkFormat format)
      : Image(device)
   {
      IMAGE_CREATE_INFO createInfo;
      createInfo.width = width;
      createInfo.height = height;
      createInfo.depth = depth;
      createInfo.format = format;
      createInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      createInfo.name = debugName;
      createInfo.finalImageLayout = VK_IMAGE_LAYOUT_GENERAL;
      createInfo.transitionToFinalLayout = true;
      CreateInternal(createInfo, device);
   }
}