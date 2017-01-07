#include "TextureLoader.h"
#include "VulkanDebug.h"
#include "VulkanDevice.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

namespace VulkanLib
{
	TextureLoader::TextureLoader(VulkanDevice* vulkanDevice, VkQueue queue)
	{
		mVulkanDevice = vulkanDevice;
		mQueue = queue;
	}

	TextureLoader::~TextureLoader()
	{
	}

	bool TextureLoader::LoadTexture(std::string filename, VulkanTexture * texture)
	{
		VkDevice device =  mVulkanDevice->GetLogicalDevice();	
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			return false;
		}

		// Create the staging image and device memory
		VkImage stagingImage = VK_NULL_HANDLE;
		VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

		CreateImage(texWidth,
					  texHeight,
					  VK_FORMAT_R8G8B8A8_UNORM,														// VkFormat
					  VK_IMAGE_TILING_LINEAR,														// VkImageTiling
					  VK_IMAGE_USAGE_TRANSFER_SRC_BIT,												// VkImageUsageFlags
					  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,	// VkMemoryPropertyFlags
					  &stagingImage,																// VkImage
					  &stagingMemory);																// VkDeviceMemory


		// Copy the pixels from the loaded image to device memory
		VkImageSubresource subresource = {};
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.arrayLayer = 0;

		VkSubresourceLayout stagingImageLayout;
		vkGetImageSubresourceLayout(device, stagingImage, &subresource, &stagingImageLayout);

		void* data;
		vkMapMemory(device, stagingMemory, 0, imageSize, 0, &data);

		if (stagingImageLayout.rowPitch == texWidth * 4) {
			memcpy(data, pixels, (size_t)imageSize);
		}
		else {
			uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

			for (int y = 0; y < texHeight; y++) {
				memcpy(&dataBytes[y * stagingImageLayout.rowPitch], &pixels[y * texWidth * 4], texWidth * 4);
			}
		}

		vkUnmapMemory(device, stagingMemory);
		stbi_image_free(pixels);

		// Create the final image
		CreateImage(texWidth,
					  texHeight,
					  VK_FORMAT_R8G8B8A8_UNORM,														// VkFormat
					  VK_IMAGE_TILING_OPTIMAL,														// VkImageTiling
					  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,					// VkImageUsageFlags
					  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,											// VkMemoryPropertyFlags
					  &texture->image,																// VkImage
					  &texture->deviceMemory);														// VkDeviceMemory

		TransitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		TransitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		CopyImage(stagingImage, texture->image, texWidth, texHeight);

		TransitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		return false;
	}

	void TextureLoader::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory)
	{
		VkDevice device =  mVulkanDevice->GetLogicalDevice();

		// Create info for the image that will be used for staging
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		VulkanDebug::ErrorCheck(vkCreateImage(device, &imageInfo, nullptr, image));

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, *image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = mVulkanDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

		VulkanDebug::ErrorCheck(vkAllocateMemory(device, &allocInfo, nullptr, imageMemory));
		VulkanDebug::ErrorCheck(vkBindImageMemory(device, *image, *imageMemory, 0));
	}

	void TextureLoader::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = mVulkanDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}
		// Extend this with more image layout transitions

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		mVulkanDevice->FlushCommandBuffer(commandBuffer, mQueue, true);
	}

	void TextureLoader::CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = mVulkanDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

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
		region.extent.width = width;
		region.extent.height = height;
		region.extent.depth = 1;

		// [NOTE] It's also possible to use vkCmdCopyBufferToImage() instead
		vkCmdCopyImage(
			commandBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region
		);

		mVulkanDevice->FlushCommandBuffer(commandBuffer, mQueue, true);
	}
}
