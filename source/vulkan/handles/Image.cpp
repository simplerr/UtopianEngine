#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/Debug.h"
#include "Image.h"
#include <stdlib.h>

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
		// Image handles all the cleanup itself
		vkDestroyImageView(GetVkDevice(), mImageView, nullptr);
		vkDestroyImage(GetVkDevice(), mHandle, nullptr);
		vkFreeMemory(GetVkDevice(), mDeviceMemory, nullptr);
	}

	void Image::CreateInternal(const IMAGE_CREATE_INFO& createInfo, Device* device)
	{
		mWidth = createInfo.width;
		mHeight = createInfo.height;
		mFormat = createInfo.format;
		mFinalImageLayout = createInfo.finalImageLayout;
		mNumMipLevels = createInfo.mipLevels;
		mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.format = createInfo.format;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent = { createInfo.width, createInfo.height, 1 };
		imageCreateInfo.mipLevels = createInfo.mipLevels;
		imageCreateInfo.arrayLayers = createInfo.arrayLayers;
		imageCreateInfo.tiling = createInfo.tiling;
		imageCreateInfo.usage = createInfo.usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.initialLayout = mCurrentLayout;

		CreateImage(imageCreateInfo, createInfo.properties);

		// Connect the view with the image
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = (createInfo.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY);
		viewCreateInfo.format = createInfo.format;
		viewCreateInfo.subresourceRange = {};
		viewCreateInfo.subresourceRange.aspectMask = createInfo.aspectFlags;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = createInfo.mipLevels;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = createInfo.arrayLayers;

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
	}

	void Image::CreateImage(VkImageCreateInfo imageCreateInfo, VkMemoryPropertyFlags properties)
	{
		Debug::ErrorCheck(vkCreateImage(GetVkDevice(), &imageCreateInfo, nullptr, &mHandle));

		// Get memory requirements
		VkMemoryRequirements memRequirments;
		vkGetImageMemoryRequirements(GetVkDevice(), mHandle, &memRequirments);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memRequirments.size;

		GetDevice()->GetMemoryType(memRequirments.memoryTypeBits, properties, &allocateInfo.memoryTypeIndex);

		Debug::ErrorCheck(vkAllocateMemory(GetVkDevice(), &allocateInfo, nullptr, &mDeviceMemory));
		Debug::ErrorCheck(vkBindImageMemory(GetVkDevice(), mHandle, mDeviceMemory, 0));
	}

	void Image::CreateView(VkImageViewCreateInfo viewCreateInfo)
	{
		viewCreateInfo.image = mHandle;
		Debug::ErrorCheck(vkCreateImageView(GetVkDevice(), &viewCreateInfo, nullptr, &mImageView));
	}

	void Image::LayoutTransition(Device* device, const CommandBuffer& commandBuffer, VkImageLayout newLayout)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mNumMipLevels;
		subresourceRange.layerCount = 1;

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

		if (mCurrentLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (mCurrentLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (mCurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (mCurrentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStageMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (mCurrentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStageMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("Unsupported layout transition!");
		}
		// Extend this with more image layout transitions

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

	void Image::SetFinalLayout(VkImageLayout finalLayout)
	{
		mFinalImageLayout = finalLayout;
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
	}

	VkFormat Image::GetFormat() const
	{
		return mFormat;
	}

	VkImageLayout Image::GetFinalLayout() const
	{
		return mFinalImageLayout;
	}

	VkDeviceMemory Image::GetDeviceMemory() const
	{
		return mDeviceMemory;
	}

	uint32_t Image::GetWidth() const
	{
		return mWidth;
	}

	uint32_t Image::GetHeight() const
	{
		return mHeight;
	}

	ImageColor::ImageColor(Device* device, uint32_t width, uint32_t height, VkFormat format, uint32_t arrayLayers)
		: Image(device)
	{
		IMAGE_CREATE_INFO createInfo;
		createInfo.width = width;
		createInfo.height = height;
		createInfo.format = format;
		createInfo.arrayLayers = arrayLayers;
		createInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		createInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		CreateInternal(createInfo, device);
	}

	ImageDepth::ImageDepth(Device* device, uint32_t width, uint32_t height, VkFormat format, uint32_t arrayLayers)
		: Image(device)
	{
		IMAGE_CREATE_INFO createInfo;
		createInfo.width = width;
		createInfo.height = height;
		createInfo.format = format;
		createInfo.arrayLayers = arrayLayers;
		createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		createInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		CreateInternal(createInfo, device);
	}
}