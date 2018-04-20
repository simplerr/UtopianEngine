#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class Image : public Handle<VkImage>
	{
	public:
		// [TODO] Make this easier to use
		// ColorImage and DepthStencilImage 
		Image(Device* device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);
		~Image();

		VkImageView GetView();
		VkFormat GetFormat();
	private:
		VkImage mImage;
		VkImageView mImageView;
		VkDeviceMemory mDeviceMemory;
		VkFormat mFormat;
	};

	class ImageColor : public Image
	{
	public:
		ImageColor(Device* device, uint32_t width, uint32_t height, VkFormat format);
	};

	class ImageDepth : public Image
	{
	public:
		ImageDepth(Device* device, uint32_t width, uint32_t height, VkFormat format);
	};
}
