#pragma once

#include <gli/gli.hpp>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Queue.h"
#include "VulkanInclude.h"
#include "vulkan/Debug.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/Utilities.h"

namespace Utopian::Vk
{
	/** @brief Cube map texture */
	class CubeMapTexture
	{
	public:
		Device* device;
		SharedPtr<Image> image;
		// VkImage image;
		// VkImageView view;
		// VkDeviceMemory deviceMemory;
		VkImageLayout imageLayout;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;

		/** @brief Optional sampler to use with this texture */
		VkSampler sampler;

		/** @brief Update image descriptor from current sampler, view and image layout */
		void UpdateDescriptor()
		{
			descriptor.sampler = sampler;
			//descriptor.imageView = view;
			descriptor.imageLayout = imageLayout;
		}

		/** @brief Release all Vulkan resources held by this texture */
		void Destroy()
		{
			if (sampler)
			{
				vkDestroySampler(device->GetVkDevice(), sampler, nullptr);
			}
		}

		/**
		* Load a cubemap texture including all mip levels from a single file
		*
		* @param filename File to load (supports .ktx and .dds)
		* @param format Vulkan format of the image data stored in the file
		* @param device Vulkan device to create the texture on
		* @param copyQueue Queue used for the texture staging copy commands (must support transfer)
		* @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
		* @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		*
		*/
		void LoadFromFile(
			std::string filename,
			VkFormat format,
			Device* device,
			Queue* copyQueue,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			gli::texture_cube texCube(gli::load(filename));

			assert(!texCube.empty());

			this->device = device;
			width = static_cast<uint32_t>(texCube.extent().x);
			height = static_cast<uint32_t>(texCube.extent().y);
			mipLevels = static_cast<uint32_t>(texCube.levels());

			// Create a host-visible staging buffer that contains the raw image data
			Buffer stagingBuffer = Buffer(device,
										  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
										  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
										  texCube.size(),
										  texCube.data());

			// Create optimal tiled target image
			VkImageCreateInfo imageCreateInfo = {};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.mipLevels = mipLevels;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.extent = { width, height, 1 };
			imageCreateInfo.usage = imageUsageFlags;
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			imageCreateInfo.arrayLayers = 6; // Cube faces count as array layers in Vulkan
			imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // This flag is required for cube map images

			// Create image view
			VkImageViewCreateInfo viewCreateInfo = {};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			viewCreateInfo.format = format;
			viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			viewCreateInfo.subresourceRange.layerCount = 6;
			viewCreateInfo.subresourceRange.levelCount = mipLevels;

			image = std::make_shared<Image>(device);
			image->CreateImage(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			image->CreateView(viewCreateInfo);

			// Setup buffer copy regions for each face including all of it's miplevels
			std::vector<VkBufferImageCopy> bufferCopyRegions;
			size_t offset = 0;

			for (uint32_t face = 0; face < 6; face++)
			{
				for (uint32_t level = 0; level < mipLevels; level++)
				{
					VkBufferImageCopy bufferCopyRegion = {};
					bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					bufferCopyRegion.imageSubresource.mipLevel = level;
					bufferCopyRegion.imageSubresource.baseArrayLayer = face;
					bufferCopyRegion.imageSubresource.layerCount = 1;
					bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(texCube[face][level].extent().x);
					bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(texCube[face][level].extent().y);
					bufferCopyRegion.imageExtent.depth = 1;
					bufferCopyRegion.bufferOffset = offset;

					bufferCopyRegions.push_back(bufferCopyRegion);

					// Increase offset into staging buffer for next level / face
					offset += texCube[face][level].size();
				}
			}

			// Image barrier for optimal image (target)
			// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = mipLevels;
			subresourceRange.layerCount = 6;

			Utilities::TransitionImageLayout(device, copyQueue, image->GetVkHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

			// Copy the cube map faces from the staging buffer to the optimal tiled image
			Utilities::CopyBufferToImage(device, copyQueue, &stagingBuffer, image.get(), bufferCopyRegions.size(), bufferCopyRegions.data());

			// Change texture image layout to shader read after all faces have been copied
			Utilities::TransitionImageLayout(device, copyQueue, image->GetVkHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageLayout, subresourceRange);

			// Create sampler
			VkSamplerCreateInfo samplerCreateInfo = {};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
			samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
			samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
			samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.anisotropyEnable = VK_TRUE;
			samplerCreateInfo.maxAnisotropy = 16;
			samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
			samplerCreateInfo.minLod = 0.0f;
			samplerCreateInfo.maxLod = (float)mipLevels;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			Debug::ErrorCheck(vkCreateSampler(device->GetVkDevice(), &samplerCreateInfo, nullptr, &sampler));

			// Update descriptor image info member that can be used for setting up descriptor sets
			UpdateDescriptor();
		}
	};

}
