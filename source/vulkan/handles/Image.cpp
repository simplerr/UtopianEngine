#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "Image.h"

namespace Utopian::Vk
{
	Image::Image(Device* device,
				 uint32_t width,
				 uint32_t height,
				 VkFormat format,
				 VkImageTiling tiling,
				 VkImageUsageFlags usage,
				 VkMemoryPropertyFlags properties,
				 VkImageAspectFlags aspectFlags)
		: Handle(device, nullptr)
	{
		mFormat = format;

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.format = format;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent = { width, height, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.usage = usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		//imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

		CreateImage(imageCreateInfo, properties);

		// Connect the view with the image
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange = {};
		viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		CreateView(viewCreateInfo);
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

	void Image::CreateImage(VkImageCreateInfo imageCreateInfo, VkMemoryPropertyFlags properties)
	{
		VulkanDebug::ErrorCheck(vkCreateImage(GetVkDevice(), &imageCreateInfo, nullptr, &mHandle));

		// Get memory requirements
		VkMemoryRequirements memRequirments;
		vkGetImageMemoryRequirements(GetVkDevice(), mHandle, &memRequirments);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memRequirments.size;

		GetDevice()->GetMemoryType(memRequirments.memoryTypeBits, properties, &allocateInfo.memoryTypeIndex);

		VulkanDebug::ErrorCheck(vkAllocateMemory(GetVkDevice(), &allocateInfo, nullptr, &mDeviceMemory));
		VulkanDebug::ErrorCheck(vkBindImageMemory(GetVkDevice(), mHandle, mDeviceMemory, 0));
	}

	void Image::CreateView(VkImageViewCreateInfo viewCreateInfo)
	{
		viewCreateInfo.image = mHandle;
		VulkanDebug::ErrorCheck(vkCreateImageView(GetVkDevice(), &viewCreateInfo, nullptr, &mImageView));
	}

	VkImageView Image::GetView()
	{
		return mImageView;
	}

	VkFormat Image::GetFormat()
	{
		return mFormat;
	}

	ImageColor::ImageColor(Device* device, uint32_t width, uint32_t height, VkFormat format)
		: Image(device, width, height, format,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT)
	{

	}

	ImageDepth::ImageDepth(Device* device, uint32_t width, uint32_t height, VkFormat format)
		: Image(device, width, height, format, 
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
	{

	}
}