#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "Image.h"

namespace Vulkan
{
	Image::Image(Device* device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags)
		: Handle(device, nullptr)
	{
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

		VulkanDebug::ErrorCheck(vkCreateImage(GetDevice(), &imageCreateInfo, nullptr, &mImage));

		// Get memory requirements
		VkMemoryRequirements memRequirments;
		vkGetImageMemoryRequirements(GetDevice(), mImage, &memRequirments);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memRequirments.size;
		device->GetMemoryType(memRequirments.memoryTypeBits, properties, &allocateInfo.memoryTypeIndex);

		VulkanDebug::ErrorCheck(vkAllocateMemory(GetDevice(), &allocateInfo, nullptr, &mDeviceMemory));
		VulkanDebug::ErrorCheck(vkBindImageMemory(GetDevice(), mImage, mDeviceMemory, 0));

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
		viewCreateInfo.image = mImage;	

		VulkanDebug::ErrorCheck(vkCreateImageView(GetDevice(), &viewCreateInfo, nullptr, &mImageView));
	}

	Image::~Image()
	{
		// Image handles all the cleanup itself
		vkDestroyImageView(GetDevice(), mImageView, nullptr);
		vkDestroyImage(GetDevice(), mImage, nullptr);
		vkFreeMemory(GetDevice(), mDeviceMemory, nullptr);
	}

	VkImageView Image::GetView()
	{
		return mImageView;
	}
}