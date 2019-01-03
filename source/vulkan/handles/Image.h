#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include <vector>

namespace Utopian::Vk
{
	class Image : public Handle<VkImage>
	{
	public:
		Image(Device* device,
			  uint32_t width,
			  uint32_t height,
			  VkFormat format,
			  VkImageTiling tiling,
			  VkImageUsageFlags usage,
			  VkMemoryPropertyFlags properties,
			  VkImageAspectFlags aspectFlags,
			  uint32_t arrayLayers = 1);

		Image(Device* device);

		// These exist so that it's possible to create images and view with non standard create infos
		// For example used by CubeMapTexture
		void CreateImage(VkImageCreateInfo imageCreateInfo, VkMemoryPropertyFlags properties);
		void CreateView(VkImageViewCreateInfo viewCreateInfo);

		~Image();

		VkImageView GetView();
		VkImageView GetLayerView(uint32_t layer);
		VkFormat GetFormat();
	private:
		// If the image has multiple layers this contains the view to each one of them
		std::vector<VkImageView> mLayerViews;

		// Contains the view to the whole image, including all layers if multiple
		VkImageView mImageView;

		VkDeviceMemory mDeviceMemory;
		VkFormat mFormat;
	};

	class ImageColor : public Image
	{
	public:
		ImageColor(Device* device, uint32_t width, uint32_t height, VkFormat format, uint32_t arrayLayers = 1);
	};

	class ImageDepth : public Image
	{
	public:
		ImageDepth(Device* device, uint32_t width, uint32_t height, VkFormat format, uint32_t arrayLayers = 1);
	};
}
