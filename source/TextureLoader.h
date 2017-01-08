#pragma once
#include <string>
#include <vulkan/vulkan.h>

namespace VulkanLib
{
	class VulkanDevice;

	struct VulkanTexture
	{
		VkImage image;
		VkDeviceMemory deviceMemory;
		VkImageView imageView;
		VkSampler sampler;

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
		TextureLoader(VulkanDevice* vulkanDevice, VkQueue queue);
		~TextureLoader();

		bool LoadTexture(std::string filename, VulkanTexture* texture);
		void DestroyTexture(VulkanTexture texture);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
		void CreateImageView(VkImage image, VkFormat format, VkImageView* imageView);
		void CreateImageSampler(VkSampler* sampler);
	private:
		VulkanDevice* mVulkanDevice;
		VkQueue mQueue;
	};
}
