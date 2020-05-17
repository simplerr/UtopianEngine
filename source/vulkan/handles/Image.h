#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include <vector>

namespace Utopian::Vk
{
	struct IMAGE_CREATE_INFO
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 1;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout finalImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		uint32_t arrayLayers = 1;
		uint32_t mipLevels = 1;
	};

	/** Wrapper for VkImage and VkImageView. */
	class Image : public Handle<VkImage>
	{
	public:
		Image(const IMAGE_CREATE_INFO& createInfo, Device* device);

		/** If specialized create infos are needed this should be called followed by CreateImage() and CreateView(). */
		Image(Device* device);

		~Image();

		/**
		 * These exist so that it's possible to create images and view with non standard create infos
		 * For example used by CubeMapTexture.
		 */
		void CreateImage(VkImageCreateInfo imageCreateInfo, VkMemoryPropertyFlags properties);
		void CreateView(VkImageViewCreateInfo viewCreateInfo);

		void LayoutTransition(Device* device, const CommandBuffer& commandBuffer, VkImageLayout newLayout);
		void Copy(CommandBuffer* commandBuffer, Image* destination);
		void SetFinalLayout(VkImageLayout finalLayout);

		void MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data);
		void UnmapMemory();
		void UpdateMemory(void* data, VkDeviceSize size);

		VkImageView GetView() const;
		VkImageView GetLayerView(uint32_t layer) const;
		VkFormat GetFormat() const;
		VkImageLayout GetFinalLayout() const;
		VkDeviceMemory GetDeviceMemory() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		VkSubresourceLayout GetSubresourceLayout(Device* device) const;
	protected:
		void CreateInternal(const IMAGE_CREATE_INFO& createInfo, Device* device);

	private:
		/** If the image has multiple layers this contains the view to each one of them. */
		std::vector<VkImageView> mLayerViews;

		/** Contains the view to the whole image, including all layers if more than one. */
		VkImageView mImageView;

		/** The image layout that's expected when used as a descriptor. */
		VkImageLayout mFinalImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkImageLayout mCurrentLayout;

		VkDeviceMemory mDeviceMemory;
		VkFormat mFormat;
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mDepth;
		uint32_t mNumMipLevels;
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
