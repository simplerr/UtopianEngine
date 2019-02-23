#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include <vector>

namespace Utopian::Vk
{
	/** Wrapper for VkImage and VkImageView. */
	class Image : public Handle<VkImage>
	{
	public:
		/** Constructor that should be used in most cases. */
		Image(Device* device,
			  uint32_t width,
			  uint32_t height,
			  VkFormat format,
			  VkImageTiling tiling,
			  VkImageUsageFlags usage,
			  VkMemoryPropertyFlags properties,
			  VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
			  uint32_t arrayLayers = 1);

		/** If specialized create infos are needed this should be called followed by CreateImage() and CreateView(). */
		Image(Device* device);

		~Image();

		/**
		 * These exist so that it's possible to create images and view with non standard create infos
		 * For example used by CubeMapTexture.
		 */
		void CreateImage(VkImageCreateInfo imageCreateInfo, VkMemoryPropertyFlags properties);
		void CreateView(VkImageViewCreateInfo viewCreateInfo);

		void SetFinalLayout(VkImageLayout imageLayout);

		VkImageView GetView() const;
		VkImageView GetLayerView(uint32_t layer) const;
		VkFormat GetFormat() const;
		VkImageLayout GetFinalLayout() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
	private:
		/** If the image has multiple layers this contains the view to each one of them. */
		std::vector<VkImageView> mLayerViews;

		/** Contains the view to the whole image, including all layers if more than one. */
		VkImageView mImageView;

		/** The image layout that's expected when used as a descriptor. */
		VkImageLayout mFinalImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDeviceMemory mDeviceMemory;
		VkFormat mFormat;
		uint32_t mWidth;
		uint32_t mHeight;
	};

	/** An image with flags corresponding to a color image. */
	class ImageColor : public Image
	{
	public:
		ImageColor(Device* device, uint32_t width, uint32_t height, VkFormat format, uint32_t arrayLayers = 1);
	};

	/** An image with flags corresponding to a depth image. */
	class ImageDepth : public Image
	{
	public:
		ImageDepth(Device* device, uint32_t width, uint32_t height, VkFormat format, uint32_t arrayLayers = 1);
	};
}
