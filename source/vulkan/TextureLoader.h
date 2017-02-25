#pragma once

#include <map>
#include <string>
#include <vulkan/vulkan.h>

#define PLACEHOLDER_TEXTURE_PATH "data/textures/checker.jpg"

namespace VulkanLib
{
	class Device;
	class DescriptorSet;
	class Renderer;

	struct VulkanTexture
	{
		VkImage image;
		VkDeviceMemory deviceMemory;
		VkImageView imageView;
		VkSampler sampler;

		DescriptorSet* descriptorSet;

		VkDescriptorImageInfo GetTextureDescriptorInfo()
		{
			VkDescriptorImageInfo texDescriptor = {};
			texDescriptor.sampler = sampler;
			texDescriptor.imageView = imageView;
			texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			return texDescriptor;
		}
	};

	class TextureLoader
	{
	public:
		TextureLoader(Renderer* renderer, VkQueue queue);
		~TextureLoader();

		VulkanTexture* LoadTexture(std::string filename);
		void DestroyTexture(VulkanTexture* texture);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
		void CreateImageView(VkImage image, VkFormat format, VkImageView* imageView);
		void CreateImageSampler(VkSampler* sampler);
	private:
		std::map<std::string, VulkanTexture*> mTextureMap;
		Renderer* mRenderer;
		Device*  mDevice;
		VkQueue mQueue;
	};
}
