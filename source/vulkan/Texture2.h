#pragma once

#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <gli/gli.hpp>
#include "vulkan/vulkan.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/Debug.h"
#include "core/renderer/RendererUtility.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	class Texture2 {
	public:
		Device* device;
		VkDescriptorImageInfo descriptor;

		SharedPtr<Vk::Image> image;
		SharedPtr<Vk::Sampler> sampler;

		void UpdateDescriptor();
	protected:
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mNumMipLevels;
		//uint32_t mLayerCount;
	};

	struct TEXTURE_CREATE_INFO
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		std::string path;
	};

	class Texture2D : public Texture2 {
	public:

		Texture2D();
		Texture2D(const TEXTURE_CREATE_INFO& createInfo, Device* device);
		Texture2D(std::string filename, Device* device);

		/**
		* Load a 2D texture including all mip levels
		*
		* @param filename File to load (supports .ktx and .dds)
		* @param format Vulkan format of the image data stored in the file
		* @param device Vulkan device to create the texture on
		* @param copyQueue Queue used for the texture staging copy commands (must support transfer)
		* @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
		* @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		*/
		void LoadFromFile(std::string filename,
						  VkFormat format,
						  Device* device,
						  VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
						  VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};
}
