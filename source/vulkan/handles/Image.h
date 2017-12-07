#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace Vulkan
{
	class Device;

	class Image : public Handle<VkImage>
	{
	public:
		// [TODO] Make this easier to use
		// ColorImage and DepthStencilImage 
		Image(Device* device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);
		~Image();

		VkImageView GetView();
	private:
		VkImage mImage;
		VkImageView mImageView;
		VkDeviceMemory mDeviceMemory;
		VkFormat mFormat;
	};
}
